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

namespace blackhole {

struct formatter_config_t {
    std::string type;
    std::map<std::string, std::string> args;
};

struct sink_config_t {
    std::string type;
    std::map<std::string, std::string> args;
};

namespace factory {

template<typename Level, typename Sink>
std::unique_ptr<base_frontend_t> create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
    if (formatter_config.type == "string") {
        typedef formatter::string_t formatter_type;

        std::string pattern = formatter_config.args.at("pattern");
        auto formatter = std::make_unique<formatter_type>(pattern);

        return std::make_unique<frontend_t<formatter_type, Sink, Level>>(std::move(formatter), std::move(sink));
    }

    return std::unique_ptr<base_frontend_t>();
}

template<typename Level>
std::unique_ptr<base_frontend_t> create(const formatter_config_t& formatter, const sink_config_t& sink) {
    if (sink.type == "files") {
        typedef sink::file_t<> sink_type;

        std::string path = sink.args.at("path");
        auto s = std::make_unique<sink_type>(path);

        return create<Level>(formatter, std::move(s));
    } else if (sink.type == "syslog") {
        typedef sink::syslog_t<Level> sink_type;

        std::string identity = sink.args.at("identity");
        auto s = std::make_unique<sink_type>(identity);

        return create<Level>(formatter, std::move(s));
    } else if (sink.type == "socket") {
        std::string type = sink.args.at("type");
        if (type == "udp") {
            typedef sink::socket_t<boost::asio::ip::udp> sink_type;
            std::string host = sink.args.at("host");
            std::uint16_t port = boost::lexical_cast<std::uint16_t>(sink.args.at("port"));
            auto s = std::make_unique<sink_type>(host, port);

            return create<Level>(formatter, std::move(s));
        } else if (type == "tcp") {
            typedef sink::socket_t<boost::asio::ip::tcp> sink_type;
            std::string host = sink.args.at("host");
            std::uint16_t port = boost::lexical_cast<std::uint16_t>(sink.args.at("port"));
            auto s = std::make_unique<sink_type>(host, port);

            return create<Level>(formatter, std::move(s));
        }
    }

    return std::unique_ptr<base_frontend_t>();
}

} // namespace factory

} // namespace blackhole

TEST(Factory, FileStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        { { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "files",
        { { "path", "/dev/stdout" } }
    };

    EXPECT_TRUE(bool(factory::create<testing::level>(formatter, sink)));
}

TEST(Factory, SyslogStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        { { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "syslog",
        { { "identity", "AppIdentity" } }
    };

    EXPECT_TRUE(bool(factory::create<testing::level>(formatter, sink)));
}

TEST(Factory, UdpSocketStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        { { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "socket", {
            { "type", "udp" },
            { "host", "localhost" },
            { "port", "50030" }
        }
    };

    EXPECT_TRUE(bool(factory::create<testing::level>(formatter, sink)));
}

TEST(Factory, TcpSocketStringsFrontend) {
    formatter_config_t formatter = {
        "string",
        { { "pattern", "[%(timestamp)s]: %(message)s" } }
    };

    sink_config_t sink = {
        "socket", {
            { "type", "tcp" },
            { "host", "localhost" },
            { "port", "50030" }
        }
    };

    EXPECT_TRUE(bool(factory::create<testing::level>(formatter, sink)));
}
