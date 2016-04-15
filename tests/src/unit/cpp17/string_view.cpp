#include <gtest/gtest.h>

#include <blackhole/cpp17/string_view.hpp>

namespace blackhole {
inline namespace v1 {
namespace cpp17 {
namespace {

TEST(string_view, CompareWithString) {
    EXPECT_EQ(string_view("message"), std::string("message"));
    EXPECT_EQ(std::string("message"), string_view("message"));
}

}  // namespace
}  // namespace cpp17
}  // namespace v1
}  // namespace blackhole
