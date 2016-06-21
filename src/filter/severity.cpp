#include "blackhole/filter/severity.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/filter.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/memory.hpp"

namespace blackhole {
inline namespace v1 {
namespace filter {

class severity_t : public filter_t {
    std::int64_t threshold;

public:
    severity_t(std::int64_t threshold) noexcept : threshold(threshold) {}

    auto filter(const record_t& record) -> filter_t::action_t override {
        if (record.severity() >= threshold) {
            return filter_t::action_t::neutral;
        } else {
            return filter_t::action_t::deny;
        }
    }
};

}  // namespace filter

auto factory<filter::severity_t>::type() const noexcept -> const char* {
    return "severity";
}

auto factory<filter::severity_t>::from(const config::node_t& config) const ->
    std::unique_ptr<filter_t>
{
    if (auto threshold = config["threshold"].to_sint64()) {
        return blackhole::make_unique<filter::severity_t>(*threshold);
    }

    throw std::invalid_argument("field 'threshold' is required");
}

}  // namespace v1
}  // namespace blackhole
