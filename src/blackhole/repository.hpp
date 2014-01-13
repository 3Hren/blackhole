#pragma once

#include <map>
#include <mutex>
#include <string>

#include "formatter/json.hpp"
#include "formatter/string.hpp"
#include "frontend.hpp"
#include "frontend/syslog.hpp"
#include "logger.hpp"
#include "repository/config/log.hpp"
#include "sink/files.hpp"
#include "sink/socket.hpp"
#include "sink/syslog.hpp"
#include "utils/unique.hpp"

namespace blackhole {

template<typename Level>
class sink_factory_t;

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

    template<class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        auto config = factory_traits<Sink>::map_config(sink_config.config);
        auto sink = std::make_unique<Sink>(config);
        return create(formatter_config, std::move(sink));
    }

    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        return sink_factory_t<Level>::instance().create(sink_config.type, formatter_config, sink_config);
    }
};

template<typename Level>
class sink_factory_t {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef std::function<return_type(const formatter_config_t&, const sink_config_t&)> factory_type;
    typedef return_type(*raw_factory_type)(const formatter_config_t&, const sink_config_t&);

    mutable std::mutex mutex;
    std::unordered_map<std::string, factory_type> sinks;
public:
    static sink_factory_t<Level>& instance() {
        static sink_factory_t<Level> self;
        return self;
    }

    template<typename T>
    void add(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
        sinks[name] = static_cast<raw_factory_type>(&factory_t<Level>::template create<T>);
    }

    return_type create(const std::string& name,
                       const formatter_config_t& formatter_config,
                       const sink_config_t& sink_config) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = sinks.find(name);
        if (it != sinks.end()) {
            return it->second(formatter_config, sink_config);
        }

        return return_type();
    }

private:
    sink_factory_t() {
        add<sink::file_t<>>("files");
        add<sink::syslog_t<Level>>("syslog");
        add<sink::socket_t<boost::asio::ip::udp>>("udp");
        add<sink::socket_t<boost::asio::ip::tcp>>("tcp");
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
