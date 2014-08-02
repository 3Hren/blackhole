#pragma once

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include "blackhole/bigbang.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/frontend.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/repository/config/defaults.hpp"
#include "blackhole/repository/config/log.hpp"
#include "blackhole/repository/factory/external.hpp"
#include "blackhole/sink/stream.hpp"

namespace blackhole {

class repository_t {
    external_factory_t factory;
    std::unordered_map<std::string, log_config_t> configs;

    mutable std::mutex mutex;

public:
    static repository_t& instance() {
        static repository_t self;
        return self;
    }

    template<typename Sink, typename Formatter>
    bool available() const {
        std::lock_guard<std::mutex> lock(mutex);
        return factory.has<Sink, Formatter>();
    }

    template<typename Sink, typename Formatter>
    void configure() {
        std::lock_guard<std::mutex> lock(mutex);
        external_inserter<Sink, Formatter>::insert(factory);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        factory.clear();
        configs.clear();
    }

    void add_config(const log_config_t& config) {
        std::lock_guard<std::mutex> lock(mutex);
        configs[config.name] = config;
    }

    void add_configs(const std::vector<log_config_t>& configs) {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto it = configs.begin(); it != configs.end(); ++it) {
            this->configs[it->name] = *it;
        }
    }

    //!@todo: Cause the repository is no longer templatized by level parameter,
    //!       it can be useful to specify logger return type explicitly.
    template<typename Level>
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

    template<typename Level>
    verbose_logger_t<Level> root() const {
        return create<Level>("root");
    }
private:
    repository_t() {
        configure<sink::stream_t, formatter::string_t>();
        add_config(repository::config::trivial());
    }
};

} // namespace blackhole

static blackhole::aux::bigbang_t bigbang;
