#include <blackhole/formatter/string.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/frontend.hpp>
#include <blackhole/frontend/files.hpp>
#include <blackhole/frontend/syslog.hpp>
#include <blackhole/repository.hpp>
#include <blackhole/sink/files.hpp>
#include <blackhole/sink/socket.hpp>
#include <blackhole/sink/stream.hpp>
#include <blackhole/sink/syslog.hpp>

#include "global.hpp"

using namespace blackhole;

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

TEST(Factory, StreamStringsFrontend) {
    external_factory_t factory;
    factory.add<sink::stream_t, formatter::string_t>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("stream");
    sink["output"] = "stdout";

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, FileStringsFrontend) {
    external_factory_t factory;
    factory.add<sink::files_t<>, formatter::string_t>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Repository, RotationFileStringsFrontendWithMoveWatcher) {
    external_factory_t factory;
    factory.add<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::move_t
            >
        >,
        formatter::string_t
    >();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;
    sink["rotation"]["move"] = true;

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Repository, RotationFileStringsFrontendWithSizeWatcher) {
    external_factory_t factory;
    factory.add<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.log.%N";
    sink["rotation"]["backups"] = 5;
    sink["rotation"]["size"] = 10 * 1024 * 1024;

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Repository, RotationFileStringsFrontendWithDatetimeWatcher) {
    external_factory_t factory;
    factory.add<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::datetime_t<>
            >
        >,
        formatter::string_t
    >();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.log.%N";
    sink["rotation"]["backups"] = 5;
    sink["rotation"]["period"] = "d";

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Repository, ThrowsExceptionIfRotationWatcherNotSpecified) {
    external_factory_t factory;
    factory.add<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::datetime_t<>
            >
        >,
        formatter::string_t
    >();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.log.%N";
    sink["rotation"]["backups"] = 5;

    EXPECT_THROW(factory.create(formatter, sink), blackhole::error_t);
}

TEST(Factory, SyslogStringsFrontend) {
    external_factory_t factory;
    factory.add<sink::syslog_t<level>, formatter::string_t>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("syslog");
    sink["identity"] = "AppIdentity";

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, UdpSocketStringsFrontend) {
    external_factory_t factory;
    factory.add<sink::socket_t<boost::asio::ip::udp>, formatter::string_t>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("udp");
    sink["host"] = "localhost";
    sink["port"] = 50030;

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

TEST(Factory, TcpSocketStringsFrontend) {
    external_factory_t factory;
    factory.add<sink::socket_t<boost::asio::ip::tcp>, formatter::string_t>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    // Sink will try to connect to the specified port. So if it isn't available,
    // an exception will be thrown, it's ok.
    sink_config_t sink("tcp");
    sink["host"] = "localhost";
    sink["port"] = 22;

    EXPECT_TRUE(bool(factory.create(formatter, sink)));
}

log_config_t create_valid_config() {
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("stream");
    sink["output"] = "stdout";

    frontend_config_t frontend = { formatter, sink };
    return log_config_t{ "root", { frontend } };
}

TEST(Repository, InitializationFromSettings) {
    log_config_t config = create_valid_config();
    repository_t::instance().add_config(config);
    const bool condition = std::is_same<verbose_logger_t<testing::level>, decltype(repository_t::instance().create<testing::level>("root"))>::value;
    static_assert(condition, "repository should return `verbose_logger_t` object");
}

TEST(Repository, ThrowsExceptionIfLoggerWithSpecifiedNameNotFound) {
    log_config_t config = create_valid_config();
    repository_t::instance().add_config(config);
    EXPECT_THROW(repository_t::instance().create<testing::level>("log"), std::out_of_range);
}

TEST(Repository, CreatesDuplicateOfRootLoggerByDefault) {
    log_config_t config = create_valid_config();
    repository_t::instance().add_config(config);
    repository_t::instance().root<testing::level>();
}

TEST(Factory, ThrowsExceptionWhenRequestNotRegisteredSink) {
    external_factory_t factory;

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;

    EXPECT_THROW(factory.create(formatter, sink), blackhole::error_t);
}

TEST(Factory, ThrowsExceptionWhenRequestNotRegisteredFormatter) {
    external_factory_t factory;
    factory.add<sink::socket_t<boost::asio::ip::udp>, boost::mpl::list<>>();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;

    EXPECT_THROW(factory.create(formatter, sink), blackhole::error_t);
}

TEST(Repository, StreamSinkWithStringFormatterIsAvailableByDefault) {
    auto& repository = repository_t::instance();
    bool available = repository.available<sink::stream_t, formatter::string_t>();
    EXPECT_TRUE(available);
    repository.clear();
}

TEST(Repository, PairConfiguring) {
    bool available = false;
    auto& repository = repository_t::instance();

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
    auto& repository = repository_t::instance();

    available = repository.available<sink::files_t<>, formatter::json_t>();
    EXPECT_FALSE(available);

    repository.configure<sink::files_t<>, formatters_t>();

    available = repository.available<sink::files_t<>, formatter::json_t>();
    EXPECT_TRUE(available);
    repository.clear();
}

TEST(Repository, CombinationConfiguring) {
    typedef boost::mpl::vector<
        sink::files_t<>,
        sink::syslog_t<level>
    > sinks_t;

    typedef boost::mpl::list<
        formatter::string_t,
        formatter::json_t
    > formatters_t;

    bool available = false;
    auto& repository = repository_t::instance();

    available = repository.available<sink::files_t<>, formatter::json_t>();
    ASSERT_FALSE(available);
    available = repository.available<sink::syslog_t<level>, formatter::string_t>();
    ASSERT_FALSE(available);
    available = repository.available<sink::syslog_t<level>, formatter::json_t>();
    ASSERT_FALSE(available);

    repository.configure<sinks_t, formatters_t>();

    available = repository.available<sink::files_t<>, formatter::json_t>();
    EXPECT_TRUE(available);
    available = repository.available<sink::syslog_t<level>, formatter::string_t>();
    EXPECT_TRUE(available);
    available = repository.available<sink::syslog_t<level>, formatter::json_t>();
    EXPECT_TRUE(available);
    repository.clear();
}

TEST(Repository, ThrowsExceptionWhenSinkIsRegisteredButItsUniqueNameIsDifferent) {
    auto& repo = repository_t::instance();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;
    sink["rotation"]["size"] = 1000;

    frontend_config_t frontend = { formatter, sink };
    log_config_t config = { "root", { frontend } };

    repo.configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();
    repo.add_config(config);

    EXPECT_THROW(repo.create<level>("root"), blackhole::error_t);
    repo.clear();
}
