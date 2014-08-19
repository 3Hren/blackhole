#include <blackhole/record.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(record_t, DefaultConstructor) {
    record_t record;

    EXPECT_FALSE(record.valid());
    EXPECT_TRUE(record.attributes().empty());
}
