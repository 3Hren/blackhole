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
    factory.add<sink::file_t<>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::vector<boost::any> {
            std::string("/dev/stdout"),
            true
        }
    };

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Repository, RotationFileStringsFrontend) {
    group_factory_t<level> factory;
    factory.add<sink::file_t<sink::boost_backend_t, sink::rotator_t>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::vector<boost::any> {
            std::string("/dev/stdout"),
            true,
            std::vector<boost::any> {
                std::uint64_t(1024),   // Size.
                std::uint16_t(3)       // Count.
            }
        }
    };

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, SyslogStringsFrontend) {
    group_factory_t<level> factory;
    factory.add<sink::syslog_t<level>, formatter::string_t>();

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
    factory.add<sink::socket_t<boost::asio::ip::udp>, formatter::string_t>();

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
    factory.add<sink::socket_t<boost::asio::ip::tcp>, formatter::string_t>();

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
        std::vector<boost::any> {
            std::string("/dev/stdout"),
            true
        }
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

TEST(Factory, ThrowsExceptionWhenRequestNotRegisteredSink) {
    group_factory_t<level> factory;

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::string("/dev/stdout")
    };

    EXPECT_THROW(factory.create(formatter, sink), blackhole::error_t);
}

TEST(Factory, ThrowsExceptionWhenRequestNotRegisteredFormatter) {
    group_factory_t<level> factory;
    factory.add<sink::socket_t<boost::asio::ip::udp>, boost::mpl::list<>>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::string("/dev/stdout")
    };

    EXPECT_THROW(factory.create(formatter, sink), blackhole::error_t);
}

TEST(Repository, FileSinkWithStringFormatterIsAvailableByDefault) {
    auto& repository = repository_t<level>::instance();
    bool available = repository.available<sink::file_t<>, formatter::string_t>();
    EXPECT_TRUE(available);
    repository.clear();
}

TEST(Repository, PairConfiguring) {
    bool available = false;
    auto& repository = repository_t<level>::instance();

    available = repository.available<sink::syslog_t<level>, formatter::string_t>();
    EXPECT_FALSE(available);

    repository.configure<sink::syslog_t<level>, formatter::string_t>();

    available = repository.available<sink::syslog_t<level>, formatter::string_t>();
    EXPECT_TRUE(available);
    repository.clear();
}

TEST(Repository, GroupConfiguring) {
    typedef boost::mpl::list<
        formatter::string_t,
        formatter::json_t
    > formatters_t;

    bool available = false;
    auto& repository = repository_t<level>::instance();

    available = repository.available<sink::file_t<>, formatter::json_t>();
    EXPECT_FALSE(available);

    repository.configure<sink::file_t<>, formatters_t>();

    available = repository.available<sink::file_t<>, formatter::json_t>();
    EXPECT_TRUE(available);
    repository.clear();
}

TEST(Repository, CombinationConfiguring) {
    typedef boost::mpl::vector<
        sink::file_t<>,
        sink::syslog_t<level>
    > sinks_t;

    typedef boost::mpl::list<
        formatter::string_t,
        formatter::json_t
    > formatters_t;

    bool available = false;
    auto& repository = repository_t<level>::instance();

    available = repository.available<sink::file_t<>, formatter::json_t>();
    ASSERT_FALSE(available);
    available = repository.available<sink::syslog_t<level>, formatter::string_t>();
    ASSERT_FALSE(available);
    available = repository.available<sink::syslog_t<level>, formatter::json_t>();
    ASSERT_FALSE(available);

    repository.configure<sinks_t, formatters_t>();

    available = repository.available<sink::file_t<>, formatter::json_t>();
    EXPECT_TRUE(available);
    available = repository.available<sink::syslog_t<level>, formatter::string_t>();
    EXPECT_TRUE(available);
    available = repository.available<sink::syslog_t<level>, formatter::json_t>();
    EXPECT_TRUE(available);
    repository.clear();
}
