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
        auto formatter = aux::util::make_unique<
            formatter::string_t
        >("[%(timestamp)s]: %(message)s");

        auto sink = aux::util::make_unique<
            sink::null_t
        >();

        auto frontend = aux::util::make_unique<
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
    wrapper_t<logger_type> wrapper(log, attribute::set_t({
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
    wrapper_t<logger_type> wrapper(log, attribute::set_t({
        attribute::make("answer", 42)
    }));

    wrapper_t<logger_type> other(std::move(wrapper));

    auto record = other.open_record();
    ASSERT_TRUE(record.attributes().find("answer"));
    EXPECT_EQ(42, record.extract<int>("answer"));
}

TEST(Wrapper, MoveAssignment) {
    /*!
     * This test checks wrapper move assignment operator.
     * Behaviour should be the same as with move constructor.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    logger_type log = logger_factory_t::create<logger_type>();
    wrapper_t<logger_type> wrapper(log, attribute::set_t({
        attribute::make("answer", 42)
    }));

    wrapper_t<logger_type> other(log, attribute::set_t({
        attribute::make("answer", 43)
    }));
    other = std::move(wrapper);

    auto record = other.open_record();
    ASSERT_TRUE(record.attributes().find("answer"));
    EXPECT_EQ(42, record.extract<int>("answer"));
}

TEST(Wrapper, Usage) {
    auto log = logger_factory_t::create<logger_base_t>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<logger_base_t> wrapper(log, attribute::set_t({
            attribute::make("answer", 42)
        }));

        auto record = log.open_record();
        ASSERT_TRUE(record.attributes().find("id"));
        EXPECT_EQ(100500, record.extract<int>("id"));
        EXPECT_FALSE(record.attributes().find("answer"));

        record = wrapper.open_record();
        ASSERT_TRUE(record.attributes().find("id"));
        EXPECT_EQ(100500, record.extract<int>("id"));
        ASSERT_TRUE(record.attributes().find("answer"));
        EXPECT_EQ(42, record.extract<int>("answer"));
    }

    auto record = log.open_record();
    ASSERT_TRUE(record.attributes().find("id"));
    EXPECT_EQ(100500, record.extract<int>("id"));
    EXPECT_FALSE(record.attributes().find("answer"));
}

TEST(Wrapper, UsageWithVerboseLogger) {
    auto log = logger_factory_t::create<verbose_logger_t<testing::level>>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<verbose_logger_t<testing::level>> wrapper(
            log,
            attribute::set_t({
                attribute::make("answer", 42)
            })
        );

        auto record = log.open_record(testing::info);
        ASSERT_TRUE(record.attributes().find("id"));
        EXPECT_EQ(100500, record.extract<int>("id"));

        ASSERT_TRUE(record.attributes().find("severity"));
        EXPECT_EQ(
            testing::info,
            record.extract<
                aux::underlying_type<testing::level>::type
            >("severity")
        );

        EXPECT_FALSE(record.attributes().find("answer"));

        record = wrapper.open_record(testing::info);
        ASSERT_TRUE(record.attributes().find("id"));
        EXPECT_EQ(100500, record.extract<int>("id"));

        ASSERT_TRUE(record.attributes().find("severity"));
        EXPECT_EQ(
            testing::info,
            record.extract<
                aux::underlying_type<testing::level>::type
            >("severity")
        );

        ASSERT_TRUE(record.attributes().find("answer"));
        EXPECT_EQ(42, record.extract<int>("answer"));
    }

    auto record = log.open_record(testing::info);
    ASSERT_TRUE(record.attributes().find("id"));
    EXPECT_EQ(100500, record.extract<int>("id"));

    ASSERT_TRUE(record.attributes().find("severity"));
    EXPECT_EQ(
        testing::info,
        record.extract<
            aux::underlying_type<testing::level>::type
        >("severity")
    );

    EXPECT_FALSE(record.attributes().find("answer"));
}

TEST(Wrapper, MacroUsage) {
    auto log = logger_factory_t::create<verbose_logger_t<testing::level>>();
    log.add_attribute(attribute::make("id", 100500));

    {
        wrapper_t<verbose_logger_t<testing::level>> wrapper(
            log,
            attribute::set_t({
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
            attribute::set_t({
                attribute::make("answer", 42)
            })
        );

        auto record = wrapper.open_record();
        ASSERT_TRUE(record.attributes().find("id"));
        EXPECT_EQ(100500, record.extract<int>("id"));
        ASSERT_TRUE(record.attributes().find("answer"));
        EXPECT_EQ(42, record.extract<int>("answer"));
        EXPECT_FALSE(record.attributes().find("result"));

        {
            wrapper_t<verbose_logger_t<testing::level>> nested(
                wrapper,
                attribute::set_t({
                    attribute::make("result", 300)
                })
            );
            auto record = nested.open_record();
            ASSERT_TRUE(record.attributes().find("id"));
            EXPECT_EQ(100500, record.extract<int>("id"));
            ASSERT_TRUE(record.attributes().find("answer"));
            EXPECT_EQ(42, record.extract<int>("answer"));
            ASSERT_TRUE(record.attributes().find("result"));
            EXPECT_EQ(300, record.extract<int>("result"));
        }

        record = wrapper.open_record();
        ASSERT_TRUE(record.attributes().find("id"));
        EXPECT_EQ(100500, record.extract<int>("id"));
        ASSERT_TRUE(record.attributes().find("answer"));
        EXPECT_EQ(42, record.extract<int>("answer"));
        EXPECT_FALSE(record.attributes().find("result"));
    }
}

TEST(Wrapper, UnderlyingLoggerType) {
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    static_assert(
        (std::is_same<logger_type, wrapper_type::logger_type>::value),
        "error in extracting underlying logger type"
    );
}

TEST(Wrapper, UnderlyingNestedLoggerType) {
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    typedef wrapper_t<wrapper_type> deep_wrapper_type;
    static_assert(
        (std::is_same<logger_type, deep_wrapper_type::logger_type>::value),
        "error in extracting underlying logger type"
    );
}

TEST(Wrapper, UnderlyingTwoLevelNestedLoggerType) {
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    typedef wrapper_t<wrapper_type> deep_wrapper_type;
    typedef wrapper_t<deep_wrapper_type> deepest_wrapper_type;
    static_assert(
        (std::is_same<logger_type, deepest_wrapper_type::logger_type>::value),
        "error in extracting underlying logger type"
    );
}

TEST(Wrapper, UnderlyingLogger) {
    /*!
     * Check underlying logger reference getter for wrapper object.
     * Opening record with this logger should result in valid record, but it
     * shouldn't contain wrapper's attributes.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    logger_type log = logger_factory_t::create<logger_type>();
    log.add_attribute(attribute::make("a", 100500));

    wrapper_type wrapper(log, attribute::set_t({attribute::make("b", 42)}));

    logger_type& initial = wrapper.log();
    auto record = initial.open_record();
    ASSERT_TRUE(record.attributes().find("a"));
    EXPECT_EQ(100500, record.extract<int>("a"));
    EXPECT_FALSE(record.attributes().find("b"));
}

TEST(Wrapper, ConstUnderlyingLogger) {
    /*!
     * Check underlying logger const reference getter for wrapper object.
     * Opening record with this logger should result in valid record, but it
     * shouldn't contain wrapper's attributes.
     */
    typedef verbose_logger_t<testing::level> logger_type;
    typedef wrapper_t<logger_type> wrapper_type;
    logger_type log = logger_factory_t::create<logger_type>();
    log.add_attribute(attribute::make("a", 100500));

    const wrapper_type wrapper(log, attribute::set_t({attribute::make("b", 42)}));

    const logger_type& initial = wrapper.log();
    auto record = initial.open_record();
    ASSERT_TRUE(record.attributes().find("a"));
    EXPECT_EQ(100500, record.extract<int>("a"));
    EXPECT_FALSE(record.attributes().find("b"));
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

    wrapper_type wrapper(log, attribute::set_t({attribute::make("b", 42)}));
    deep_wrapper_type deep_wrapper(wrapper, attribute::set_t({attribute::make("c", 5)}));

    logger_type& initial = deep_wrapper.log();
    auto record = initial.open_record();
    ASSERT_TRUE(record.attributes().find("a"));
    EXPECT_EQ(100500, record.extract<int>("a"));
    EXPECT_FALSE(record.attributes().find("b"));
    EXPECT_FALSE(record.attributes().find("c"));
}

TEST(Wrapper, ConstUnderlyingNestedLogger) {
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

    wrapper_type wrapper(log, attribute::set_t({attribute::make("b", 42)}));
    const deep_wrapper_type deep_wrapper(wrapper, attribute::set_t({attribute::make("c", 5)}));

    const logger_type& initial = deep_wrapper.log();
    auto record = initial.open_record();
    ASSERT_TRUE(record.attributes().find("a"));
    EXPECT_EQ(100500, record.extract<int>("a"));
    EXPECT_FALSE(record.attributes().find("b"));
    EXPECT_FALSE(record.attributes().find("c"));
}
