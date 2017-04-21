#include <gtest/gtest.h>

#include <src/formatter/string/token.hpp>

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace string {
namespace {

TEST(name, SurroundSpecWithBraces) {
    EXPECT_EQ("{:<20}", ph::attribute<name>("<20").format);
}

} // namespace
} // namespace string
} // namespace formatter
} // namespace v1
} // namespace blackhole
