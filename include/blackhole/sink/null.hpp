#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {

class config_t;

template<typename>
struct factory;

}  // namespace blackhole

namespace blackhole {
namespace sink {

/// A null sink merely exists, it never outputs a message to any device.
///
/// This class exists primarily for benchmarking reasons to measure the entire logging processing
/// pipeline. It never fails and never throws, because it does nothing.
///
/// All methods of this class are thread safe.
class null_t : public sink_t {
public:
    /// Returns `false` regardless of a log record value, causing any log event to be dropped.
    auto filter(const record_t& record) -> bool;

    /// Drops any incoming log event.
    auto execute(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink

template<>
struct factory<sink::null_t> {
    static auto from(const config_t& config) -> sink::null_t;
};

}  // namespace blackhole
