#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/sink/socket/udp.hpp>

namespace blackhole {
namespace testing {
namespace sink {

TEST(udp_t, type) {
    EXPECT_EQ("udp", std::string(blackhole::factory<blackhole::sink::socket::udp_t>::type()));
}

}  // namespace sink
}  // namespace testing
}  // namespace blackhole
