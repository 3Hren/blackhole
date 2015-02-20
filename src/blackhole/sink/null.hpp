#pragma once

#include <string>

#include "blackhole/config.hpp"

#include "blackhole/repository/factory/traits.hpp"
#include "blackhole/sink/thread.hpp"

BLACKHOLE_BEG_NS

namespace sink {

namespace null {

struct config_t {};

} // namespace null

class null_t {
public:
    typedef null::config_t config_type;

    null_t() {}
    null_t(const config_type&) {}

    static const char* name() {
        return "null";
    }

    void consume(const std::string&) {}
};

template<>
struct thread_safety<null_t> :
    public std::integral_constant<
        thread::safety_t,
        thread::safety_t::safe
    >::type
{};

} // namespace sink

template<>
struct factory_traits<sink::null_t> {
    typedef sink::null_t sink_type;
    typedef sink_type::config_type config_type;
    static void map_config(const aux::extractor<sink_type>&, config_type&) {}
};

BLACKHOLE_END_NS
