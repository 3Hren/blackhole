#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {
namespace sink {

class console_t : public sink_t {
public:
    auto filter(const record_t&) -> bool;
    auto execute(const record_t&, const string_view& formatted) -> void;
};

}  // namespace sink
}  // namespace blackhole
