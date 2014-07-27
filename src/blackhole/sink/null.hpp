#pragma once

#include <string>

#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

namespace sink {

namespace null {

struct config_t {};

} // namespace null

class null_t {
public:
    typedef null::config_t config_type;

    //!@todo: Thread safe!

    static const char* name() {
        return "null";
    }

    null_t() {}
    null_t(const config_type&) {}

    void consume(const std::string&) {}
};

} // namespace sink

template<>
struct factory_traits<sink::null_t> {
    typedef sink::null_t sink_type;
    typedef sink_type::config_type config_type;
    static void map_config(const aux::extractor<sink_type>&, config_type&) {}
};

} // namespace blackhole
