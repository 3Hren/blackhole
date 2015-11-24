#include <blackhole/config/none.hpp>

#include <gtest/gtest.h>

namespace blackhole {
namespace testing {

using config::none_t;

TEST(null_t, IsNil) {
    none_t config;

    EXPECT_TRUE(config.is_nil());
    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_i64());
    EXPECT_FALSE(config.is_u64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

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

}  // namespace testing
}  // namespace blackhole
