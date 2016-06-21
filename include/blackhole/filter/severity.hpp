#pragma once

#include "blackhole/factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace filter {

class severity_t;

}  // namespace filter

template<>
class factory<filter::severity_t> : public factory<filter_t> {
public:
    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<filter_t> override;
};

}  // namespace v1
}  // namespace blackhole
