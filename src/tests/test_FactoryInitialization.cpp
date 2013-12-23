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
    }
};

} } // namespace blackhole::sink


TEST(Factory, FileStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        std::map<std::string, std::string>{ { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "files",
        std::map<std::string, std::string>{ { "path", "/dev/stdout" } }
    };

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
}

TEST(Factory, SyslogStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        std::map<std::string, std::string>{ { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "syslog",
        std::map<std::string, std::string>{ { "identity", "AppIdentity" } }
    };

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
}

TEST(Factory, UdpSocketStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        std::map<std::string, std::string>{ { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "socket", std::map<std::string, std::string>{
            { "type", "udp" },
            { "host", "localhost" },
            { "port", "50030" }
        }
    };

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
}

TEST(Factory, TcpSocketStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        std::map<std::string, std::string>{ { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    // Sink will try to connect to the specified port. So if it isn't available, an exception will be thrown, it's ok.
    sink_config_t sink = {
        "socket", std::map<std::string, std::string>{
            { "type", "tcp" },
            { "host", "localhost" },
            { "port", "22" }
        }
    };

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
}

log_config_t create_valid_config() {
    formatter_config_t formatter = {
        "string",
        std::map<std::string, std::string>{ { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "files",
        std::map<std::string, std::string>{ { "path", "/dev/stdout" } }
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
