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

namespace blackhole {
inline namespace v1 {

namespace {

namespace ph = std::placeholders;

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
        auto formatter = this->formatter(result<const config::node_t&>(config["formatter"].unwrap())
            .expect("each handler must have a formatter"));

        std::vector<std::unique_ptr<sink_t>> sinks;
        config["sinks"].each([&](const config::node_t& config) {
            sinks.emplace_back(this->sink(config));
        });

        auto handler = this->handler(config);
        handler->set(std::move(formatter));

        for (auto& sink : sinks) {
            handler->add(std::move(sink));
        }

        handlers.emplace_back(std::move(handler));
    });

    return root_logger_t(std::move(handlers));
}

auto builder_t::sink(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    const auto type = result<std::string>(config["type"].to_string())
        .expect("sink must have a type");

    return registry.sink(type)(config);
}

auto builder_t::handler(const config::node_t& config) const -> std::unique_ptr<handler_t> {
    const auto type = config["type"].to_string()
        .get_value_or("blocking");

    return registry.handler(type)(config);
}

auto builder_t::formatter(const config::node_t& config) const -> std::unique_ptr<formatter_t> {
    const auto type = result<std::string>(config["type"].to_string())
        .expect("formatter must have a type");

    return registry.formatter(type)(config);
}

auto registry_t::configured() -> registry_t {
    registry_t registry;

    // registry.add<formatter::string_t>();

    // registry.add<sink::console_t>();
    registry.add<sink::null_t>();
    // registry.add<sink::syslog_t>();

    // registry.add<handler::blocking_t>();

    return registry;
}

auto registry_t::add(std::shared_ptr<factory<sink_t>> fn) -> void {
    sinks[fn->type()] = std::bind(&factory<sink_t>::from, fn, ph::_1);
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
