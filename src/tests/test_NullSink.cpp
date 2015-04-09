#include <gtest/gtest.h>

#include <blackhole/sink/null.hpp>

#include "util/unused.hpp"

using namespace blackhole;

TEST(null_t, Class) {
    sink::null::config_t config;
    sink::null_t sink(config);
    UNUSED(sink);
}

TEST(null_t, IsThreadSafe) {
    static_assert(
        sink::thread_safety<
            sink::null_t
        >::type::value == sink::thread::safety_t::safe,
        "`null_t` sink must be thread safe"
    );
}
