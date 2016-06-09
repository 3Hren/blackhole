#pragma once

#include "blackhole/factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class syslog_t;

}  // namespace sink

template<>
class factory<sink::syslog_t> : public factory<sink_t> {
public:
    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace v1
}  // namespace blackhole
