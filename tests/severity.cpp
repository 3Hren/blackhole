#include <gtest/gtest.h>

#include <blackhole/severity.hpp>

namespace blackhole {
namespace testing {

TEST(severity_t, FromInt) {
    severity_t severity(4);
    EXPECT_EQ(4, severity);
}

}  // namespace testing
}  // namespace blackhole
