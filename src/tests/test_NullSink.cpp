#include "Mocks.hpp"

TEST(null_t, Class) {
    sink::null::config_t config;
    sink::null_t sink(config);
    UNUSED(sink);
}
