#pragma once

#include <map>
#include <mutex>
#include <string>

#include <boost/mpl/for_each.hpp>

#include "formatter/json.hpp"
#include "formatter/string.hpp"
#include "frontend.hpp"
#include "frontend/syslog.hpp"
#include "logger.hpp"
#include "repository/config/log.hpp"
#include "repository/factory/group.hpp"
#include "repository/registrator.hpp"
#include "sink/files.hpp"
#include "sink/socket.hpp"
#include "sink/syslog.hpp"
#include "utils/unique.hpp"

namespace blackhole {

template<typename Level>
class repository_t {
    mutable std::mutex mutex;
    group_factory_t<Level> factory;
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
            auto frontend = factory.create(frontend_config.formatter, frontend_config.sink);
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
        typedef boost::mpl::vector<
            sink::file_t<>,
            sink::syslog_t<Level>,
            sink::socket_t<boost::asio::ip::udp>,
            sink::socket_t<boost::asio::ip::tcp>
        > sinks_t;

        typedef boost::mpl::list<
            formatter::string_t,
            formatter::json_t
        > formatters_t;

        factory.template add<sink::file_t<>, formatters_t>();
        factory.template add<sink::syslog_t<Level>, formatters_t>();
        factory.template add<sink::socket_t<boost::asio::ip::udp>, formatters_t>();
        factory.template add<sink::socket_t<boost::asio::ip::tcp>, formatters_t>();

        init(make_trivial_config());
    }

    static log_config_t make_trivial_config() {
        formatter_config_t formatter = {
            "string",
            std::string("[%(timestamp)s] [%(severity)s]: %(message)s")
        };

        sink_config_t sink = {
            "files",
            std::string("/dev/stdout")
        };

        frontend_config_t frontend = { formatter, sink };
        return log_config_t{ "trivial", { frontend } };
    }
};

} // namespace blackhole
