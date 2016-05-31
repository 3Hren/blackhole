#include "blackhole/registry.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/factory.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/extensions/format.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/handler/blocking.hpp"
#include "blackhole/root.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/console.hpp"
#include "blackhole/sink/null.hpp"
#include "blackhole/sink/syslog.hpp"
#include "blackhole/factory.hpp"

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
    factory(std::move(factory))
{}

builder_t::~builder_t() = default;

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

auto registry_t::configured() -> registry_t {
    registry_t registry;

    registry.add_<formatter::string_t>();

    registry.add_<sink::console_t>();
    registry.add_<sink::null_t>();
    registry.add_<sink::syslog_t>();

    registry.add_<handler::blocking_t>(registry); // TODO: Unsafe, pray for copy elision. Return unique_ptr instead.

    return registry;
}

auto registry_t::add(std::shared_ptr<experimental::factory<sink_t>> factory) -> void {
    sinks[factory->type()] = [=](const config::node_t& node) {
        return factory->from(node);
    };
}

auto registry_t::add(std::shared_ptr<experimental::factory<handler_t>> factory) -> void {
    handlers[factory->type()] = [=](const config::node_t& node) {
        return factory->from(node);
    };
}

auto registry_t::add(std::shared_ptr<experimental::factory<formatter_t>> factory) -> void {
    formatters[factory->type()] = [=](const config::node_t& node) {
        return factory->from(node);
    };
}

auto registry_t::sink(const std::string& type) const -> sink_factory {
    return get(&sinks, type)
        .expect<std::out_of_range>(R"(sink with type "{}" is not registered)", type);
}

auto registry_t::handler(const std::string& type) const -> handler_factory {
    return get(&handlers, type)
        .expect<std::out_of_range>(R"(handler with type "{}" is not registered)", type);
}

auto registry_t::formatter(const std::string& type) const -> formatter_factory {
    return get(&formatters, type)
        .expect<std::out_of_range>(R"(formatter with type "{}" is not registered)", type);
}

}  // namespace v1
}  // namespace blackhole
