#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/logger/wrapper.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/null.hpp>

#include "global.hpp"

using namespace blackhole;

namespace {

class logger_factory_t {
public:
    template<class Logger>
    static Logger create() {
        Logger logger;
        auto formatter = utils::make_unique<
            formatter::string_t
        >("[%(timestamp)s]: %(message)s");

        auto sink = utils::make_unique<
            sink::null_t
        >();

        auto frontend = utils::make_unique<
            frontend_t<
                formatter::string_t,
                sink::null_t
            >
        >(std::move(formatter), std::move(sink));

        logger.add_frontend(std::move(frontend));
        return logger;
    }
};

} // namespace

TEST(Wrapper, Class) {
    typedef verbose_logger_t<testing::level> logger_type;
    logger_type log;
    wrapper_t<logger_type> wrapper(log, log::attributes_t({
        attribute::make("answer", 42)
    }));
    UNUSED(wrapper);
}

TEST(Wrapper, MoveConstructor) {
    /*!
     * This test checks wrapper move constructor.
     * After moving, first wrapper becomes invalid. Any action done with it
     * results in assertion.
     * Attached attributes should migrate to the new parent.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    logger_type log = logger_factory_t::create<logger_type>();
    wrapper_t<logger_type> wrapper(log, log::attributes_t({
        attribute::make("answer", 42)
    }));

    wrapper_t<logger_type> other(std::move(wrapper));

    auto record = other.open_record();
    ASSERT_EQ(1, record.attributes.count("answer"));
    EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
}

TEST(Wrapper, MoveAssignment) {
    /*!
     * This test checks wrapper move assignment operator.
     * Behaviour should be the same as with move constructor.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    logger_type log = logger_factory_t::create<logger_type>();
    wrapper_t<logger_type> wrapper(log, log::attributes_t({
        attribute::make("answer", 42)
    }));

    wrapper_t<logger_type> other(log, log::attributes_t({
        attribute::make("answer", 43)
    }));
    other = std::move(wrapper);

    auto record = other.open_record();
    ASSERT_EQ(1, record.attributes.count("answer"));
    EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
}

TEST(Wrapper, Usage) {
    auto log = logger_factory_t::create<logger_base_t>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<logger_base_t> wrapper(log, log::attributes_t({
            attribute::make("answer", 42)
        }));

        auto record = log.open_record();
        ASSERT_EQ(1, record.attributes.count("id"));
        EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);
        EXPECT_EQ(0, record.attributes.count("answer"));

        record = wrapper.open_record();
        ASSERT_EQ(1, record.attributes.count("id"));
        EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);
        ASSERT_EQ(1, record.attributes.count("answer"));
        EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
    }

    auto record = log.open_record();
    ASSERT_EQ(1, record.attributes.count("id"));
    EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);
    EXPECT_EQ(0, record.attributes.count("answer"));
}

TEST(Wrapper, UsageWithVerboseLogger) {
    auto log = logger_factory_t::create<verbose_logger_t<testing::level>>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<verbose_logger_t<testing::level>> wrapper(
            log,
            log::attributes_t({
                attribute::make("answer", 42)
            })
        );

        auto record = log.open_record(testing::info);
        ASSERT_EQ(1, record.attributes.count("id"));
        EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);

        ASSERT_EQ(1, record.attributes.count("severity"));
        EXPECT_EQ(testing::info,
                  boost::get<
                        aux::underlying_type<testing::level>::type
                  >(record.attributes["severity"].value)
                );

        EXPECT_EQ(0, record.attributes.count("answer"));

        record = wrapper.open_record(testing::info);
        ASSERT_EQ(1, record.attributes.count("id"));
        EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);

        ASSERT_EQ(1, record.attributes.count("severity"));
        EXPECT_EQ(testing::info,
                  boost::get<
                        aux::underlying_type<testing::level>::type
                  >(record.attributes["severity"].value)
                );

        ASSERT_EQ(1, record.attributes.count("answer"));
        EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
    }

    auto record = log.open_record(testing::info);
    ASSERT_EQ(1, record.attributes.count("id"));
    EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);

    ASSERT_EQ(1, record.attributes.count("severity"));
    EXPECT_EQ(testing::info,
              boost::get<
                    aux::underlying_type<testing::level>::type
              >(record.attributes["severity"].value)
            );

    EXPECT_EQ(0, record.attributes.count("answer"));
}

TEST(Wrapper, MacroUsage) {
    auto log = logger_factory_t::create<verbose_logger_t<testing::level>>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<verbose_logger_t<testing::level>> wrapper(
            log,
            log::attributes_t({
                attribute::make("answer", 42)
            })
        );

        BH_LOG(wrapper, testing::info, "everything is bad");
    }
}

TEST(Wrapper, NestedWrappers) {
    auto log = logger_factory_t::create<verbose_logger_t<testing::level>>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<verbose_logger_t<testing::level>> wrapper(
            log,
            log::attributes_t({
                attribute::make("answer", 42)
            })
        );

        auto record = wrapper.open_record();
        ASSERT_EQ(1, record.attributes.count("id"));
        EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);
        ASSERT_EQ(1, record.attributes.count("answer"));
        EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
        EXPECT_EQ(0, record.attributes.count("result"));

        {
            wrapper_t<verbose_logger_t<testing::level>> nested(
                wrapper,
                log::attributes_t({
                    attribute::make("result", 300)
                })
            );
            auto record = nested.open_record();
            ASSERT_EQ(1, record.attributes.count("id"));
            EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);
            ASSERT_EQ(1, record.attributes.count("answer"));
            EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
            EXPECT_EQ(1, record.attributes.count("result"));
            EXPECT_EQ(log::attribute_value_t(300), record.attributes["result"].value);
        }

        record = wrapper.open_record();
        ASSERT_EQ(1, record.attributes.count("id"));
        EXPECT_EQ(log::attribute_value_t(100500), record.attributes["id"].value);
        ASSERT_EQ(1, record.attributes.count("answer"));
        EXPECT_EQ(log::attribute_value_t(42), record.attributes["answer"].value);
        EXPECT_EQ(0, record.attributes.count("result"));
    }
}

TEST(Wrapper, UnderlyingLoggerType) {
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    static_assert(
        std::is_same<logger_type, wrapper_type::logger_type>::value,
        "error in extracting underlying logger type"
    );
}

TEST(Wrapper, UnderlyingNestedLoggerType) {
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    typedef wrapper_t<wrapper_type> deep_wrapper_type;
    static_assert(
        std::is_same<logger_type, deep_wrapper_type::logger_type>::value,
        "error in extracting underlying logger type"
    );
}

TEST(Wrapper, UnderlyingTwoLevelNestedLoggerType) {
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    typedef wrapper_t<wrapper_type> deep_wrapper_type;
    typedef wrapper_t<deep_wrapper_type> deepest_wrapper_type;
    static_assert(
        std::is_same<logger_type, deepest_wrapper_type::logger_type>::value,
        "error in extracting underlying logger type"
    );
}

TEST(Wrapper, UnderlyingLogger) {
    /*!
     * Check underlying logger getter for wrapper object.
     * Opening record with this logger should result in valid record, but it
     * shouldn't contain wrapper's attributes.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    logger_type log = logger_factory_t::create<logger_type>();
    log.add_attribute(attribute::make("a", 100500));

    wrapper_type wrapper(log, log::attributes_t({attribute::make("b", 42)}));

    logger_type& initial = wrapper.log();
    auto record = initial.open_record();
    ASSERT_EQ(1, record.attributes.count("a"));
    EXPECT_EQ(log::attribute_value_t(100500), record.attributes["a"].value);
    EXPECT_EQ(0, record.attributes.count("b"));
}

TEST(Wrapper, UnderlyingNestedLogger) {
    /*!
     * Check initial underlying logger getter for wrapper object wrapped with
     * another wrapper (2-level wrapper).
     * Opening record with this logger should result in valid record, but it
     * should contain none of both wrapper's attributes.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    typedef wrapper_t<wrapper_type> deep_wrapper_type;
    logger_type log = logger_factory_t::create<logger_type>();
    log.add_attribute(attribute::make("a", 100500));

    wrapper_type wrapper(log, log::attributes_t({attribute::make("b", 42)}));
    deep_wrapper_type deep_wrapper(wrapper, log::attributes_t({attribute::make("c", 5)}));

    logger_type& initial = deep_wrapper.log();
    auto record = initial.open_record();
    ASSERT_EQ(1, record.attributes.count("a"));
    EXPECT_EQ(log::attribute_value_t(100500), record.attributes["a"].value);
    EXPECT_EQ(0, record.attributes.count("b"));
    EXPECT_EQ(0, record.attributes.count("c"));
}
