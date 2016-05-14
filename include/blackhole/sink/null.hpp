#pragma once

#include "blackhole/factory.hpp"
#include "blackhole/forward.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

/// A null sink merely exists, it never outputs a message to any device.
///
/// This class exists primarily for benchmarking reasons to measure the entire logging processing
/// pipeline. It never fails and never throws, because it does nothing.
///
/// \remark All methods of this class are thread safe.
class null_t;

}  // namespace sink

template<>
struct factory<sink::null_t> {
    [[deprecated]]
    static auto type() -> const char*;

    [[deprecated]]
    static auto from(const config::node_t& config) -> sink::null_t;
};

namespace experimental {

template<>
class factory<sink::null_t> : public experimental::factory<sink_t> {
public:
    auto type() const noexcept -> const char* override;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
