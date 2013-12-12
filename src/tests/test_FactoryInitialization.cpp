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

//!@todo: Decompose it into separate file.
namespace blackhole {

struct formatter_config_t {
    std::string type;
    std::map<std::string, std::string> args;
};

struct sink_config_t {
    std::string type;
    std::map<std::string, std::string> args;
};

template<typename Level>
struct factory_t {
    template<typename Formatter, typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) {
        return std::make_unique<frontend_t<Formatter, Sink, Level>>(std::move(formatter), std::move(sink));
    }

    template<typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        if (formatter_config.type == "string") {
            std::string pattern = formatter_config.args.at("pattern");
            auto formatter = std::make_unique<formatter::string_t>(pattern);
            return create(std::move(formatter), std::move(sink));
        }

        return std::unique_ptr<base_frontend_t>();
    }

    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        if (sink_config.type == "files") {
            std::string path = sink_config.args.at("path");
            auto sink = std::make_unique<sink::file_t<>>(path);
            return create(formatter_config, std::move(sink));
        } else if (sink_config.type == "syslog") {
            std::string identity = sink_config.args.at("identity");
            auto sink = std::make_unique<sink::syslog_t<Level>>(identity);
            return create(formatter_config, std::move(sink));
        } else if (sink_config.type == "socket") {
            std::string type = sink_config.args.at("type");
            if (type == "udp") {
                std::string host = sink_config.args.at("host");
                std::uint16_t port = boost::lexical_cast<std::uint16_t>(sink_config.args.at("port"));
                auto sink = std::make_unique<sink::socket_t<boost::asio::ip::udp>>(host, port);
                return create(formatter_config, std::move(sink));
            } else if (type == "tcp") {
                std::string host = sink_config.args.at("host");
                std::uint16_t port = boost::lexical_cast<std::uint16_t>(sink_config.args.at("port"));
                auto sink = std::make_unique<sink::socket_t<boost::asio::ip::tcp>>(host, port);
                return create(formatter_config, std::move(sink));
            }
        }

        return std::unique_ptr<base_frontend_t>();
    }
};

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

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
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

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
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

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
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

    EXPECT_TRUE(bool(factory_t<testing::level>::create(formatter, sink)));
}
