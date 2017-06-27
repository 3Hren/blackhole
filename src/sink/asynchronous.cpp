#include "blackhole/sink/asynchronous.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/registry.hpp"

#include "../memory.hpp"
#include "../util/deleter.hpp"
#include "asynchronous.hpp"

namespace blackhole {
inline namespace v1 {

class builder<sink::asynchronous_t>::inner_t {
public:
    std::unique_ptr<sink_t> wrapped;
    std::unique_ptr<sink::overflow_policy_t> overflow_policy;
    std::size_t factor;
};

builder<sink::asynchronous_t>::builder(std::unique_ptr<sink_t> wrapped) :
    d(new inner_t{std::move(wrapped), sink::overflow_policy_factory_t().create("wait"), 10})
{}

auto builder<sink::asynchronous_t>::factor(std::size_t value) & -> builder& {
    d->factor = value;
    return *this;
}

auto builder<sink::asynchronous_t>::factor(std::size_t value) && -> builder&& {
    return std::move(factor(value));
}

auto builder<sink::asynchronous_t>::wait() & -> builder& {
    d->overflow_policy = sink::overflow_policy_factory_t().create("wait");
    return *this;
}

auto builder<sink::asynchronous_t>::drop() && -> builder&& {
    return std::move(wait());
}

auto builder<sink::asynchronous_t>::drop() & -> builder& {
    d->overflow_policy = sink::overflow_policy_factory_t().create("drop");
    return *this;
}

auto builder<sink::asynchronous_t>::wait() && -> builder&& {
    return std::move(wait());
}

auto builder<sink::asynchronous_t>::build() && -> std::unique_ptr<sink_t> {
    return blackhole::make_unique<sink::asynchronous_t>(std::move(d->wrapped), d->factor);
}

auto factory<sink::asynchronous_t>::type() const noexcept -> const char* {
    return "asynchronous";
}

auto factory<sink::asynchronous_t>::from(const config::node_t& config) const ->
    std::unique_ptr<sink_t>
{
    auto type = config["sink"]["type"].to_string();

    if (!type) {
        throw std::invalid_argument("\"sink\" field with \"type\" is required");
    }

    auto factory = registry.sink(type.get());

    auto factor = config["factor"].to_uint64().get();
    auto overflow = sink::overflow_policy_factory_t().create(config["overflow"].to_string().get());

    // It's safe to unwrap here, because we've already checked that there is "sink" child and it's
    // an object.
    auto sink = factory(*config["sink"].unwrap());

    return std::unique_ptr<sink_t>(new sink::asynchronous_t(std::move(sink), factor, std::move(overflow)));
}

template auto deleter_t::operator()(builder<sink::asynchronous_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
