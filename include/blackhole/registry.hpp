#pragma once

#include <functional>
#include <memory>
#include <string>

#include "forward.hpp"

namespace blackhole {
inline namespace v1 {

class builder_t {
    const registry_t& registry;
    std::unique_ptr<config::factory_t, deleter_t> factory;

public:
    builder_t(const registry_t& registry, std::unique_ptr<config::factory_t> factory);

    auto configurator() noexcept -> config::factory_t&;

    auto build(const std::string& name) -> root_logger_t;

private:
    auto handler(const config::node_t& config) const -> std::unique_ptr<handler_t>;
};

class registry_t {
public:
    typedef std::function<std::unique_ptr<sink_t>(const config::node_t&)> sink_factory;
    typedef std::function<std::unique_ptr<handler_t>(const config::node_t&)> handler_factory;
    typedef std::function<std::unique_ptr<formatter_t>(const config::node_t&)> formatter_factory;

public:
    virtual ~registry_t() = default;

    /// Returns the sink factory with the given type if registered, throws otherwise.
    virtual auto sink(const std::string& type) const -> sink_factory = 0;

    /// Returns the handler factory with the given type if registered, throws otherwise.
    virtual auto handler(const std::string& type) const -> handler_factory = 0;

    /// Returns the formatter factory with the given type if registered, throws otherwise.
    virtual auto formatter(const std::string& type) const -> formatter_factory = 0;

    /// Returns a logger builder by constructing its configuration factory using the given
    /// arguments.
    template<typename T, typename... Args>
    auto builder(Args&&... args) const -> builder_t;

    /// Registers a new typed factory with this registry.
    ///
    /// After registering the new factory can be used for constructing formatters, sinks or handlers
    /// depending on type using generic configuration object.
    template<typename T, typename... Args>
    auto add(Args&&... args) -> void;

    /// Registers a new sink factory with this registry.
    virtual auto add(std::shared_ptr<factory<sink_t>> factory) -> void = 0;

    /// Registers a new handler factory with this registry.
    virtual auto add(std::shared_ptr<factory<handler_t>> factory) -> void = 0;

    /// Registers a new formatter factory with this registry.
    virtual auto add(std::shared_ptr<factory<formatter_t>> factory) -> void = 0;
};

template<typename T, typename... Args>
inline auto registry_t::add(Args&&... args) -> void {
    add(std::make_shared<factory<T>>(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
inline auto registry_t::builder(Args&&... args) const -> builder_t {
    return {*this, config::factory_traits<T>::construct(std::forward<Args>(args)...)};
}

namespace registry {

/// Creates a new empty default registry.
auto empty() -> std::unique_ptr<registry_t>;

/// Creates a new configured default registry, which will contain all available components in the
/// library.
auto configured() -> std::unique_ptr<registry_t>;

}  // namespace registry
}  // namespace v1
}  // namespace blackhole
