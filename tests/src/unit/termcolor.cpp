#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/termcolor.hpp>

namespace blackhole {
inline namespace v1 {
namespace {

TEST(termcolor_t, Default) {
    std::ostringstream stream;
    stream << termcolor_t();

    EXPECT_EQ("\033[39m", stream.str());
}

TEST(termcolor_t, Red) {
    std::ostringstream stream;
    stream << termcolor_t::red();

    EXPECT_EQ("\033[31m", stream.str());
}

TEST(termcolor_t, Green) {
    std::ostringstream stream;
    stream << termcolor_t::green();

    EXPECT_EQ("\033[32m", stream.str());
}

TEST(termcolor_t, Yellow) {
    std::ostringstream stream;
    stream << termcolor_t::yellow();

    EXPECT_EQ("\033[33m", stream.str());
}

TEST(termcolor_t, Blue) {
    std::ostringstream stream;
    stream << termcolor_t::blue();

    EXPECT_EQ("\033[34m", stream.str());
}

TEST(termcolor_t, Reset) {
    std::ostringstream stream;
    stream << termcolor_t::reset();

    EXPECT_EQ("\033[0m", stream.str());
}

}  // namespace
}  // namespace v1
}  // namespace blackhole
