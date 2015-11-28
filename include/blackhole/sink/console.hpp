#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {

class config_t;

template<typename>
struct factory;

}  // namespace blackhole

namespace blackhole {
namespace sink {

class console_t : public sink_t {
public:
    auto filter(const record_t& record) -> bool;
    auto execute(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink

template<>
struct factory<sink::console_t> {
    static auto type() -> const char*;
    static auto from(const config_t& config) -> sink::console_t;
};

}  // namespace blackhole
