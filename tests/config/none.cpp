#include <blackhole/config/none.hpp>

#include <gtest/gtest.h>

namespace blackhole {
namespace testing {

using config::none_t;

TEST(null_t, ThrowsOnEveryGetterInvocation) {
    none_t config;

    EXPECT_THROW(config.to_bool(), bad_optional_access);
    EXPECT_THROW(config.to_i64(), bad_optional_access);
    EXPECT_THROW(config.to_u64(), bad_optional_access);
    EXPECT_THROW(config.to_double(), bad_optional_access);
    EXPECT_THROW(config.to_string(), bad_optional_access);

    EXPECT_THROW(config.each({}), bad_optional_access);
    EXPECT_THROW(config.each_map({}), bad_optional_access);
}

TEST(null_t, ChainSubscript) {
}

}  // namespace testing
}  // namespace blackhole
