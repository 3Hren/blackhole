#pragma once

#include <map>
#include <string>

#include "formatter/string.hpp"
#include "frontend.hpp"
#include "sink/files.hpp"
#include "sink/socket.hpp"
#include "sink/syslog.hpp"
#include "utils/unique.hpp"

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
