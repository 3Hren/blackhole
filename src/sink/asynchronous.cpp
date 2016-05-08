#include "blackhole/sink/asynchronous.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/registry.hpp"

#include "blackhole/detail/sink/asynchronous.hpp"

namespace blackhole {
inline namespace v1 {
namespace experimental {

auto factory<sink::asynchronous_t>::type() const noexcept -> const char* {
    return "asynchronous";
}

auto factory<sink::asynchronous_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    auto factory = registry.sink(*config["sink"]["type"].to_string());
    return std::unique_ptr<sink_t>(new sink::asynchronous_t(factory(*config["sink"].unwrap())));
}

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
