#include "blackhole/registry.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/factory.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/formatter/string.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/handler/blocking.hpp"
#include "blackhole/root.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/console.hpp"
#include "blackhole/sink/null.hpp"

namespace blackhole {

builder_t::builder_t(const registry_t& registry, std::unique_ptr<config::factory_t> factory) :
    registry(registry),
    factory(std::move(factory))
{}

builder_t::~builder_t() = default;

auto builder_t::build(const std::string& name) -> root_logger_t {
    const auto& config = factory->config();

    std::vector<std::unique_ptr<handler_t>> handlers;

    config[name].each([&](const config::node_t& config) {
        auto formatter = this->formatter(config["formatter"].unwrap());

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
    return registry.sink(*config["type"].to_string())(config);
}

auto builder_t::handler(const config::node_t& config) const -> std::unique_ptr<handler_t> {
    return registry.handler(config["type"].to_string().value_or("blocking"))(config);
}

auto builder_t::formatter(const config::node_t& config) const -> std::unique_ptr<formatter_t> {
    return registry.formatter(*config["type"].to_string())(config);
}

auto registry_t::configured() -> registry_t {
    registry_t registry;

    registry.add<formatter::string_t>();

    registry.add<sink::console_t>();
    registry.add<sink::null_t>();

    registry.add<handler::blocking_t>();

    return registry;
}

auto registry_t::sink(const std::string& type) const -> sink_factory {
    return sinks.at(type);
}

auto registry_t::handler(const std::string& type) const -> handler_factory {
    return handlers.at(type);
}

auto registry_t::formatter(const std::string& type) const -> formatter_factory {
    return formatters.at(type);
}

}  // namespace blackhole
