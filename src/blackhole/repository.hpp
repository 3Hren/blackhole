#pragma once

#include <map>
#include <mutex>
#include <string>

#include "formatter/json.hpp"
#include "formatter/string.hpp"
#include "frontend.hpp"
#include "frontend/syslog.hpp"
#include "logger.hpp"
#include "sink/files.hpp"
#include "sink/socket.hpp"
#include "sink/syslog.hpp"
#include "utils/unique.hpp"

namespace blackhole {

struct formatter_config_t {
    std::string type;
    boost::any config;
    mapping::mapper_t mapper;

    formatter_config_t() {}

    formatter_config_t(const std::string& type, const boost::any& config) :
        type(type),
        config(config)
    {}

    formatter_config_t(const std::string& type, const boost::any& config, const mapping::mapper_t& mapper) :
        type(type),
        config(config),
        mapper(mapper)
    {}
};

struct sink_config_t {
    std::string type;
    std::map<std::string, std::string> args;
};

struct frontend_config_t {
    formatter_config_t formatter;
    sink_config_t sink;
};

struct log_config_t {
    std::string name;
    std::vector<frontend_config_t> frontends;
};

template<typename Level>
struct factory_t {
    template<typename Formatter, typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) {
        return std::make_unique<frontend_t<Formatter, Sink, Level>>(std::move(formatter), std::move(sink));
    }

    template<typename Formatter, typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        auto config = factory_traits<Formatter>::map_config(formatter_config.config);
        auto formatter = std::make_unique<Formatter>(config);
        formatter->set_mapper(formatter_config.mapper);
        return create(std::move(formatter), std::move(sink));
    }

    template<typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        if (formatter_config.type == "string") {
            return create<formatter::string_t>(formatter_config, std::move(sink));
        } else if (formatter_config.type == "json") {
            return create<formatter::json_t>(formatter_config, std::move(sink));
        }

        return std::unique_ptr<base_frontend_t>();
    }

    template<class Sink, typename... Args>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, Args&&... args) {
        auto sink = std::make_unique<Sink>(std::forward<Args>(args)...);
        return create(formatter_config, std::move(sink));
    }

    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        if (sink_config.type == "files") {
            std::string path = sink_config.args.at("path");
            return create<sink::file_t<>>(formatter_config, path);
        } else if (sink_config.type == "syslog") {
            std::string identity = sink_config.args.at("identity");
            return create<sink::syslog_t<Level>>(formatter_config, identity);
        } else if (sink_config.type == "socket") {
            std::string type = sink_config.args.at("type");
            if (type == "udp") {
                std::string host = sink_config.args.at("host");
                std::uint16_t port = boost::lexical_cast<std::uint16_t>(sink_config.args.at("port"));
                return create<sink::socket_t<boost::asio::ip::udp>>(formatter_config, host, port);
            } else if (type == "tcp") {
                std::string host = sink_config.args.at("host");
                std::uint16_t port = boost::lexical_cast<std::uint16_t>(sink_config.args.at("port"));
                return create<sink::socket_t<boost::asio::ip::tcp>>(formatter_config, host, port);
            }
        }

        return std::unique_ptr<base_frontend_t>();
    }
};

template<typename Level>
class repository_t {
    mutable std::mutex mutex;
    std::unordered_map<std::string, log_config_t> configs;

public:
    static repository_t& instance() {
        static repository_t self;
        return self;
    }

    void init(log_config_t config) {
        std::lock_guard<std::mutex> lock(mutex);
        configs[config.name] = config;
    }

    verbose_logger_t<Level> create(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);
        log_config_t config = configs.at(name);
        verbose_logger_t<Level> logger;
        for (auto it = config.frontends.begin(); it != config.frontends.end(); ++it) {
            const frontend_config_t& frontend_config = *it;
            auto frontend = factory_t<Level>::create(frontend_config.formatter, frontend_config.sink);
            logger.add_frontend(std::move(frontend));
        }
        return logger;
    }

    verbose_logger_t<Level> root() const {
        return create("root");
    }

    verbose_logger_t<Level> trivial() const {
        return create("trivial");
    }

private:
    repository_t() {
        init(make_trivial_config());
    }

    static log_config_t make_trivial_config() {
        formatter_config_t formatter = {
            "string",
            std::string("[%(timestamp)s] [%(severity)s]: %(message)s")
        };

        std::map<std::string, std::string> sink_args = { { "path", "/dev/stdout" } };
        sink_config_t sink = {
            "files",
            sink_args
        };

        frontend_config_t frontend = { formatter, sink };
        return log_config_t{ "trivial", { frontend } };
    }
};

} // namespace blackhole
