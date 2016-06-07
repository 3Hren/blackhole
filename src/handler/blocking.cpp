#include "blackhole/handler/blocking.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/registry.hpp"
#include "blackhole/sink.hpp"

#include "blackhole/detail/memory.hpp"
#include "blackhole/detail/util/deleter.hpp"

#include "blocking.hpp"

namespace blackhole {
inline namespace v1 {
namespace handler {

blocking_t::blocking_t(std::unique_ptr<formatter_t> formatter,
                       std::vector<std::unique_ptr<sink_t>> sinks) :
    formatter(std::move(formatter)),
    sinks(std::move(sinks))
{}

auto blocking_t::handle(const record_t& record) -> void {
    writer_t writer;

    formatter->format(record, writer);

    for (const auto& sink : sinks) {
        // TODO: Check for filter.
        sink->emit(record, writer.result());
    }
}

}  // namespace handler

namespace experimental {

using handler::blocking_t;

class builder<blocking_t>::inner_t {
public:
    std::unique_ptr<formatter_t> formatter;
    std::vector<std::unique_ptr<sink_t>> sinks;
};

// TODO: TEST!
builder<blocking_t>::builder() :
    d(new inner_t)
{}

auto builder<blocking_t>::set(std::unique_ptr<formatter_t> formatter) & -> builder& {
    d->formatter = std::move(formatter);
    return *this;
}

auto builder<blocking_t>::set(std::unique_ptr<formatter_t> formatter) && -> builder&& {
    return std::move(set(std::move(formatter)));
}

auto builder<blocking_t>::add(std::unique_ptr<sink_t> sink) & -> builder& {
    d->sinks.emplace_back(std::move(sink));
    return *this;
}

auto builder<blocking_t>::add(std::unique_ptr<sink_t> sink) && -> builder&& {
    return std::move(add(std::move(sink)));
}

auto builder<blocking_t>::build() && -> std::unique_ptr<handler_t> {
    return blackhole::make_unique<blocking_t>(std::move(d->formatter), std::move(d->sinks));
}

auto factory<blocking_t>::type() const noexcept -> const char* {
    return "blocking";
}

auto factory<blocking_t>::from(const config::node_t& config) const -> std::unique_ptr<handler_t> {
    builder<blocking_t> builder;

    // TODO: Unsafe! Test and wrap with result.
    if (auto type = config["formatter"]["type"].to_string()) {
        builder.set(registry.formatter(type.get())(*config["formatter"].unwrap()));
    } else {
        throw std::invalid_argument("each handler must have a formatter with type");
    }

    config["sinks"].each([&](const config::node_t& config) {
        if (auto type = config["type"].to_string()) {
            builder.add(registry.sink(type.get())(config));
        } else {
            throw std::invalid_argument("each sink must have a type");
        }
    });

    return std::move(builder).build();
}

}  // namespace experimental

template auto deleter_t::operator()(experimental::builder<handler::blocking_t>::inner_t*) -> void;

}  // namespace v1
}  // namespace blackhole
