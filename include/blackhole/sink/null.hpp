#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {

class config_t;

template<typename>
struct factory;

}  // namespace blackhole

namespace blackhole {
namespace sink {

/// Null sink implementation that drops all incoming events.
class null_t : public sink_t {
public:
    auto filter(const record_t& record) -> bool;
    auto execute(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink

template<>
struct factory<sink::null_t> {
    static auto from(const config_t& config) -> sink::null_t;
};

}  // namespace blackhole
