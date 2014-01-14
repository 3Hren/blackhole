#include "Mocks.hpp"

namespace testing {

enum class level : std::uint8_t { debug, info, warn, error };

} // namespace testing

namespace blackhole { namespace sink {

template<>
struct priority_traits<testing::level> {
    static inline
    priority_t map(testing::level lvl) {
        switch (lvl) {
        case testing::level::debug:
            return priority_t::debug;
        case testing::level::info:
            return priority_t::info;
        case testing::level::warn:
            return priority_t::warning;
        case testing::level::error:
            return priority_t::err;
        }

        return priority_t::debug;
    }
};

} } // namespace blackhole::sink


TEST(Factory, FileStringsFrontend) {
    group_factory_t<level> factory;
    factory.template add<sink::file_t<>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::string("/dev/stdout")
    };

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, SyslogStringsFrontend) {
    group_factory_t<level> factory;
    factory.template add<sink::syslog_t<level>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "syslog",
        std::string("AppIdentity")
    };

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, UdpSocketStringsFrontend) {
    group_factory_t<level> factory;
    factory.template add<sink::socket_t<boost::asio::ip::udp>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "udp",
        std::vector<boost::any> {
            std::string("localhost"),
            std::uint16_t(50030)
        }
    };

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, TcpSocketStringsFrontend) {
    group_factory_t<level> factory;
    factory.template add<sink::socket_t<boost::asio::ip::tcp>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    // Sink will try to connect to the specified port. So if it isn't available,
    // an exception will be thrown, it's ok.
    sink_config_t sink = {
        "tcp",
        std::vector<boost::any> {
            std::string("localhost"),
            std::uint16_t(22)
        }
    };

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

log_config_t create_valid_config() {
    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::string("/dev/stdout")
    };

    frontend_config_t frontend = { formatter, sink };
    return log_config_t{ "root", { frontend } };
}

TEST(Repository, InitializationFromSettings) {
    log_config_t config = create_valid_config();
    repository_t<testing::level>::instance().init(config);
    const bool condition = std::is_same<verbose_logger_t<testing::level>, decltype(repository_t<testing::level>::instance().create("root"))>::value;
    static_assert(condition, "repository should return `verbose_logger_t` object");
}

TEST(Repository, ThrowsExceptionIfLoggerWithSpecifiedNameNotFound) {
    log_config_t config = create_valid_config();
    repository_t<testing::level>::instance().init(config);
    EXPECT_THROW(repository_t<testing::level>::instance().create("log"), std::out_of_range);
}

TEST(Repository, CreatesDuplicateOfRootLoggerByDefault) {
    log_config_t config = create_valid_config();
    repository_t<testing::level>::instance().init(config);
    repository_t<testing::level>::instance().root();
}

TEST(FactoryTraits, StringFormatterConfig) {
    formatter_config_t config = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    auto actual = factory_traits<formatter::string_t>::map_config(config.config);

    EXPECT_EQ("[%s]: %s", actual.pattern);
    EXPECT_EQ(std::vector<std::string>({ "timestamp", "message" }), actual.attribute_names);
}

TEST(FactoryTraits, JsonFormatterConfig) {
    formatter_config_t config = {
        "json",
        boost::any {
            std::vector<boost::any> {
                true,
                std::unordered_map<std::string, std::string> { { "message", "@message" } },
                std::unordered_map<std::string, boost::any> {
                    { "/", std::vector<std::string> { "message" } },
                    { "/fields", std::string("*") }
                }
            }
        }
    };

    formatter::json::config_t actual = factory_traits<formatter::json_t>::map_config(config.config);

    using namespace formatter::json::map;
    EXPECT_TRUE(actual.newline);
    ASSERT_TRUE(actual.naming.find("message") != actual.naming.end());
    EXPECT_EQ("@message", actual.naming["message"]);

    typedef std::unordered_map<std::string, positioning_t::positions_t> specified_positioning_t;
    ASSERT_TRUE(actual.positioning.specified.find("message") != actual.positioning.specified.end());
    EXPECT_EQ(std::vector<std::string>({}), actual.positioning.specified["message"]);
    EXPECT_EQ(std::vector<std::string>({ "fields" }), actual.positioning.unspecified);
}

TEST(Factory, ThrowsExceptionWhenRequestNotRegisteredFormatter) {
    group_factory_t<level> factory;

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::string("/dev/stdout")
    };

    EXPECT_THROW(factory.create(formatter, sink), error_t);
}
