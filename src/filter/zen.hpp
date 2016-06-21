#pragma once

#include "blackhole/filter.hpp"

namespace blackhole {
inline namespace v1 {
namespace filter {

class zen_t : public filter_t {
public:
    auto filter(const record_t&) -> filter_t::action_t override {
        return filter_t::action_t::neutral;
    }
};

}  // namespace filter
}  // namespace v1
}  // namespace blackhole
