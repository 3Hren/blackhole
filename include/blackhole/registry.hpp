#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "factory.hpp"

namespace blackhole {
inline namespace v1 {

class formatter_t;
class handler_t;
class sink_t;

class root_logger_t;

namespace config {

class factory_t;
class node_t;

template<typename>
class factory;

}  // namespace config

class registry_t;

class builder_t {
    const registry_t& registry;
    std::unique_ptr<config::factory_t> factory;

public:
    builder_t(const registry_t& registry, std::unique_ptr<config::factory_t> factory);
    builder_t(const builder_t& other) = delete;
    builder_t(builder_t&& other) = default;

    ~builder_t();

    // TODO: Implement. This requires the signature of formatter factories to be changed to accept
    // severity_map argument.
    // auto sevmap(severity_map sevmap) -> void;

    auto build(const std::string& name) -> root_logger_t;

    // TODO: Hide?
    auto configurator() -> config::factory_t& {
        return *factory;
    }

private:
    auto sink(const config::node_t& config) const -> std::unique_ptr<sink_t>;
    auto handler(const config::node_t& config) const -> std::unique_ptr<handler_t>;
    auto formatter(const config::node_t& config) const -> std::unique_ptr<formatter_t>;
};

class registry_t {
public:
    typedef std::function<std::unique_ptr<sink_t>(const config::node_t&)> sink_factory;
    typedef std::function<std::unique_ptr<handler_t>(const config::node_t&)> handler_factory;
    typedef std::function<std::unique_ptr<formatter_t>(const config::node_t&)> formatter_factory;

private:
    std::map<std::string, sink_factory> sinks;
    std::map<std::string, handler_factory> handlers;
    std::map<std::string, formatter_factory> formatters;

public:
    static auto configured() -> registry_t;

    /// Returns a logger builder by constructing its configuration factory using the given
    /// arguments.
    template<typename T, typename... Args>
    auto builder(Args&&... args) const -> builder_t;

    /// Registers a new typed factory with this registry.
    ///
    /// After registering the new factory can be used for constructing formatters, sinks or handlers
    /// depending on type using generic configuration object.
    template<typename T>
    auto add(factory<T> f = factory<T>()) ->
        typename std::enable_if<std::is_same<T, typename factory<T>::sink_type>::value>::type
    {
        sinks[f.type()] = std::bind(&factory<T>::from, std::move(f), std::placeholders::_1);
    }

    template<typename T>
    auto add(factory<T> f = factory<T>()) ->
        typename std::enable_if<std::is_same<T, typename factory<T>::handler_type>::value>::type
    {
        handlers[f.type()] = std::move(f);
    }

    template<typename T>
    auto add(factory<T> f = factory<T>()) ->
        typename std::enable_if<std::is_same<T, typename factory<T>::formatter_type>::value>::type
    {
        formatters[f.type()] = std::move(f);
    }

    /// Returns the sink factory with the given type if registered, throws otherwise.
    auto sink(const std::string& type) const -> sink_factory;

    /// Returns the handler factory with the given type if registered, throws otherwise.
    auto handler(const std::string& type) const -> handler_factory;

    /// Returns the formatter factory with the given type if registered, throws otherwise.
    auto formatter(const std::string& type) const -> formatter_factory;
};

template<typename T, typename... Args>
inline auto
registry_t::builder(Args&&... args) const -> builder_t {
    return {*this, std::unique_ptr<config::factory<T>>(new config::factory<T>(std::forward<Args>(args)...))};
}

}  // namespace v1
}  // namespace blackhole
