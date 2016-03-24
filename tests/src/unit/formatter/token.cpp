#include <gtest/gtest.h>

#include <blackhole/detail/formatter/string/token.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {
namespace {

TEST(name, SurroundSpecWithBraces) {
    EXPECT_EQ("{:<20}", ph::attribute<name>("<20").format);
}

}  // namespace
}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
