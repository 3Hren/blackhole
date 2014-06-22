#include <blackhole/formatter/string.hpp>
#include <blackhole/log.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/logger/wrapper.hpp>
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
    verbose_logger_t<testing::level> log;
    wrapper_t<verbose_logger_t<testing::level>> wrapper(log, log::attributes_t({
        attribute::make("answer", 42)
    }));
    UNUSED(wrapper);
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
