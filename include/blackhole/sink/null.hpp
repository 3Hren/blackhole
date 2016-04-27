#pragma once

#include "../factory.hpp"
#include "../sink.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

/// A null sink merely exists, it never outputs a message to any device.
///
/// This class exists primarily for benchmarking reasons to measure the entire logging processing
/// pipeline. It never fails and never throws, because it does nothing.
///
/// \remark All methods of this class are thread safe.
class null_t : public sink_t {
public:
    /// Drops any incoming log event.
    auto emit(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink

template<>
class factory<sink::null_t> : public factory<sink_t> {
public:
    virtual auto type() const noexcept -> const char* override;
    virtual auto from(const config::node_t& config) const -> std::unique_ptr<sink_t> override;
};

}  // namespace v1
}  // namespace blackhole
