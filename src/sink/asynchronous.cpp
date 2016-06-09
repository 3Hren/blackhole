#include "blackhole/sink/asynchronous.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/registry.hpp"

#include "blackhole/detail/sink/asynchronous.hpp"

namespace blackhole {
inline namespace v1 {

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

}  // namespace v1
}  // namespace blackhole
