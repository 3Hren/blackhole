#include "blackhole/registry.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/factory.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/factory.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/root.hpp"
#include "blackhole/sink.hpp"

#include "blackhole/detail/memory.hpp"

#include "essentials.hpp"

namespace blackhole {
inline namespace v1 {

namespace {

template<typename T>
class result {
    boost::optional<T> value;

public:
    result() {}
    result(const T& value) : value(value) {}
    result(boost::optional<T> value) : value(std::move(value)) {}

    template<typename E = std::logic_error, typename... Args>
    auto expect(const char* message, const Args&... args) -> T {
        if (value) {
            return *value;
        } else {
            throw E(fmt::format(message, args...));
        }
    }
};

template<typename V>
auto get(const std::map<std::string, V>* map, const std::string& key) -> result<V> {
    try {
        return {map->at(key)};
    } catch (const std::out_of_range&) {
        return {};
    }
}

}  // namespace

builder_t::builder_t(const registry_t& registry, std::unique_ptr<config::factory_t> factory) :
    registry(registry),
    factory(factory.release())
{}

auto builder_t::configurator() noexcept -> config::factory_t& {
    return *factory;
}

auto builder_t::build(const std::string& name) -> root_logger_t {
    const auto& config = factory->config();

    std::vector<std::unique_ptr<handler_t>> handlers;

    // TODO: Check `config.contains(name)`.
    config[name].each([&](const config::node_t& config) {
        handlers.emplace_back(handler(config));
    });

    return root_logger_t(std::move(handlers));
}

auto builder_t::handler(const config::node_t& config) const -> std::unique_ptr<handler_t> {
    const auto type = config["type"].to_string()
        .get_value_or("blocking");

    return registry.handler(type)(config);
}

class default_registry_t : public registry_t {
public:
    typedef registry_t::sink_factory sink_factory;
    typedef registry_t::handler_factory handler_factory;
    typedef registry_t::formatter_factory formatter_factory;

private:
    std::map<std::string, sink_factory> sinks;
    std::map<std::string, handler_factory> handlers;
    std::map<std::string, formatter_factory> formatters;

public:
    auto sink(const std::string& type) const -> sink_factory override;
    auto handler(const std::string& type) const -> handler_factory override;
    auto formatter(const std::string& type) const -> formatter_factory override;

    auto add(std::shared_ptr<factory<sink_t>> factory) -> void override;
    auto add(std::shared_ptr<factory<handler_t>> factory) -> void override;
    auto add(std::shared_ptr<factory<formatter_t>> factory) -> void override;
};

auto default_registry_t::add(std::shared_ptr<factory<sink_t>> factory) -> void {
    sinks[factory->type()] = [=](const config::node_t& node) {
        return factory->from(node);
    };
}

auto default_registry_t::add(std::shared_ptr<factory<handler_t>> factory) -> void {
    handlers[factory->type()] = [=](const config::node_t& node) {
        return factory->from(node);
    };
}

auto default_registry_t::add(std::shared_ptr<factory<formatter_t>> factory) -> void {
    formatters[factory->type()] = [=](const config::node_t& node) {
        return factory->from(node);
    };
}

auto default_registry_t::sink(const std::string& type) const -> sink_factory {
    return get(&sinks, type)
        .expect<std::out_of_range>(R"(sink with type "{}" is not registered)", type);
}

auto default_registry_t::handler(const std::string& type) const -> handler_factory {
    return get(&handlers, type)
        .expect<std::out_of_range>(R"(handler with type "{}" is not registered)", type);
}

auto default_registry_t::formatter(const std::string& type) const -> formatter_factory {
    return get(&formatters, type)
        .expect<std::out_of_range>(R"(formatter with type "{}" is not registered)", type);
}

auto registry::configured() -> std::unique_ptr<registry_t> {
    std::unique_ptr<registry_t> registry(new default_registry_t);
    essentials(*registry);

    return registry;
}

auto registry::empty() -> std::unique_ptr<registry_t> {
    return blackhole::make_unique<default_registry_t>();
}

}  // namespace v1
}  // namespace blackhole
