#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {
inline namespace v1 {

template<typename>
struct factory;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole

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
struct factory<sink::null_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::null_t;
};

}  // namespace v1
}  // namespace blackhole
