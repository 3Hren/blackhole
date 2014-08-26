#pragma once

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

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
    /*!
     * Default constructor.
     * After creation registeres string/stream frontend, which makes possible
     * to create logger with that frontend type.
     * Also adds trivial logger configuration.
     * @post: registered<sink::stream_t, formatter::string_t>() == true.
     * @post: create<T>("root") - should create valid logger.
     */
    repository_t() {
        configure<sink::stream_t, formatter::string_t>();
        add_config(repository::config::trivial());
    }

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

    template<typename Level>
    typename std::enable_if<
        std::is_enum<Level>::value,
        verbose_logger_t<Level>
    >::type
    root() const {
        return create<Level>("root");
    }

    /*!
     * @deprecated[soft]: since 0.3.
     * @deprecated[hard]: will be removed since 0.4.
     */
    template<typename Level>
    typename std::enable_if<
        std::is_enum<Level>::value,
        verbose_logger_t<Level>
    >::type
    create(const std::string& name) const {
        return create<verbose_logger_t<Level>>(name);
    }

    /*!
     * @since: 0.3
     */
    template<class Logger>
    typename std::enable_if<
        std::is_base_of<logger_base_t, Logger>::value,
        Logger
    >::type
    create(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex);

        const auto& frontends = configs.at(name).frontends;
        Logger logger;
        for (auto it = frontends.begin(); it != frontends.end(); ++it) {
            logger.add_frontend(factory.create(it->formatter, it->sink));
        }
        return logger;
    }
};

} // namespace blackhole
