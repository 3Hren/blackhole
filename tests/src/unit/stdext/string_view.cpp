#include <gtest/gtest.h>

#include <blackhole/stdext/string_view.hpp>

namespace blackhole {
inline namespace v1 {
namespace stdext {
namespace {

TEST(string_view, CompareWithString) {
    EXPECT_EQ(string_view("message"), std::string("message"));
    EXPECT_EQ(std::string("message"), string_view("message"));
}

}  // namespace
}  // namespace stdext
}  // namespace v1
}  // namespace blackhole
