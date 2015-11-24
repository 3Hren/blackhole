#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {
namespace sink {

/// Null sink implementation that drops all incoming events.
class null_t : public sink_t {
public:
    auto filter(const record_t& record) -> bool;
    auto execute(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink
}  // namespace blackhole
