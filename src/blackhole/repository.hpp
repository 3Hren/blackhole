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
#include "blackhole/repository/factory.hpp"
#include "blackhole/sink/stream.hpp"

namespace blackhole {

class repository_t {
    factory_t factory;
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
    repository_t();

    static repository_t& instance();

    /*!
     * Check whether some sink-formatter pair is registered with this
     * repository.
     * Registered sink-formatter pair composes into a frontend, which can be
     * potentially injected inside the logger. Trying to create a logger with
     * unregistered frontend results in exception.
     * @since: 0.3.
     */
    template<typename Sink, typename Formatter>
    bool registered() const;

    /*!
     * Alias for 'registered' method.
     * @see: repository_t::registered().
     * @deprecated[soft]: since 0.3.
     * @deprecated[hard]: will be removed since 0.4.
     */
    template<typename Sink, typename Formatter>
    bool available() const BLACKHOLE_DEPRECATED("use 'registered' instead");

    /*!
     * Register sink-formatter pair (which is a frontend itself) with the
     * repository.
     * Registered frontends can be injected inside the logger while its
     * creation stage if the logger's configuration requires them.
     * @post: registered<Sink, Formatter>() == true.
     */
    template<typename Sink, typename Formatter>
    void registrate();

    /*!
     * Alias for 'registrate' method.
     * @see: repository_t::registrate.
     * @deprecated[soft]: since 0.3.
     * @deprecated[hard]: will be removed since 0.4.
     */
    template<typename Sink, typename Formatter>
    void configure() BLACKHOLE_DEPRECATED("use 'registrate' instead");

    /*!
     * Clear all registered frontend configurations as like as logger configs.
     * @post: registered<Sink, Formatter>() == false on any combinations of
     *        sinks and formatters.
     * @deprecated[soft]: since 0.3.
     * @deprecated[hard]: will be removed since 0.4.
     */
    void clear() BLACKHOLE_DEPRECATED("there is no need for this method");

    /*!
     * Add generic logger configuration into the repository.
     * After that logger with name 'config.name' can be created using 'create'
     * method.
     * By default 'root' logger is registered with string-stream frontend.
     */
    void add_config(const log_config_t& config);

    /*!
     * Add several generic logger configurations into the repository.
     */
    void add_configs(const std::vector<log_config_t>& configs);

    /*!
     * Create logger from configuration named 'root', obviously.
     * @deprecated[soft]: since 0.3.
     * @deprecated[hard]: will be removed since 0.4.
     */
    template<typename Level>
    typename std::enable_if<
        std::is_enum<Level>::value,
        verbose_logger_t<Level>
    >::type
    root() const BLACKHOLE_DEPRECATED("use 'create<T>(\"root\")' instead");

    /*!
     * Create verbose logger with specified configuration name.
     * @deprecated[soft]: since 0.3.
     * @deprecated[hard]: will be removed since 0.4.
     */
    template<typename Level>
    typename std::enable_if<
        std::is_enum<Level>::value,
        verbose_logger_t<Level>
    >::type
    BLACKHOLE_DEPRECATED("use 'create<T>(name)' instead")
    create(const std::string& name) const;

    /*!
     * Create typed logger with specified configuration name.
     * To successfull logger creation its configuration named 'root' should be
     * added previously into the repository as like as its frontends must be
     * registered with.
     * @since: 0.3.
     */
    template<class Logger>
    typename std::enable_if<
        std::is_base_of<logger_base_t, Logger>::value,
        Logger
    >::type
    create(const std::string& name) const;
};

BLACKHOLE_API
repository_t::repository_t() {
    //registrate<sink::stream_t, formatter::string_t>();
    add_config(repository::config::trivial());
}

BLACKHOLE_API
repository_t&
repository_t::instance() {
    static repository_t self;
    return self;
}

template<typename Sink, typename Formatter>
BLACKHOLE_API
bool
repository_t::registered() const {
    std::lock_guard<std::mutex> lock(mutex);
    return factory.has<Sink, Formatter>();
}

template<typename Sink, typename Formatter>
BLACKHOLE_API
bool
repository_t::available() const {
    return registered<Sink, Formatter>();
}

template<typename Sink, typename Formatter>
BLACKHOLE_API
void
repository_t::registrate() {
    std::lock_guard<std::mutex> lock(mutex);
    factory.add<Sink, Formatter>();
}

template<typename Sink, typename Formatter>
BLACKHOLE_API
void
repository_t::configure() {
    registrate<Sink, Formatter>();
}

BLACKHOLE_API
void
repository_t::clear() {
    std::lock_guard<std::mutex> lock(mutex);
    factory.clear();
    configs.clear();
}

BLACKHOLE_API
void
repository_t::add_config(const log_config_t& config) {
    std::lock_guard<std::mutex> lock(mutex);
    configs[config.name] = config;
}

BLACKHOLE_API
void
repository_t::add_configs(const std::vector<log_config_t>& configs) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto it = configs.begin(); it != configs.end(); ++it) {
        this->configs[it->name] = *it;
    }
}

template<typename Level>
BLACKHOLE_API
typename std::enable_if<
    std::is_enum<Level>::value,
    verbose_logger_t<Level>
>::type
repository_t::root() const {
    return create<verbose_logger_t<Level>>("root");
}

template<typename Level>
BLACKHOLE_API
typename std::enable_if<
    std::is_enum<Level>::value,
    verbose_logger_t<Level>
>::type
repository_t::create(const std::string& name) const {
    return create<verbose_logger_t<Level>>(name);
}

template<class Logger>
BLACKHOLE_API
typename std::enable_if<
    std::is_base_of<logger_base_t, Logger>::value,
    Logger
>::type
repository_t::create(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex);

    const auto& frontends = configs.at(name).frontends;
    Logger logger;
    for (auto it = frontends.begin(); it != frontends.end(); ++it) {
        logger.add_frontend(factory.create(it->formatter, it->sink));
    }
    return logger;
}

} // namespace blackhole
