#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/record.hpp>
#include <blackhole/sink/console.hpp>

namespace blackhole {
namespace testing {
namespace sink {

using ::testing::StrictMock;
using ::testing::internal::CaptureStdout;
using ::testing::internal::GetCapturedStdout;

using ::blackhole::sink::color_t;
using ::blackhole::sink::console_t;

TEST(color_t, Default) {
    std::ostringstream stream;
    stream << color_t();

    EXPECT_EQ("\033[39m", stream.str());
}

TEST(color_t, Red) {
    std::ostringstream stream;
    stream << color_t::red();

    EXPECT_EQ("\033[31m", stream.str());
}

TEST(color_t, Green) {
    std::ostringstream stream;
    stream << color_t::green();

    EXPECT_EQ("\033[32m", stream.str());
}

TEST(color_t, Yellow) {
    std::ostringstream stream;
    stream << color_t::yellow();

    EXPECT_EQ("\033[33m", stream.str());
}

TEST(color_t, Blue) {
    std::ostringstream stream;
    stream << color_t::blue();

    EXPECT_EQ("\033[34m", stream.str());
}

TEST(color_t, Reset) {
    std::ostringstream stream;
    stream << color_t::reset();

    EXPECT_EQ("\033[0m", stream.str());
}

TEST(console_t, WriteIntoStandardOutputByDefault) {
    console_t sink;

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    CaptureStdout();
    sink.execute(record, "expected");

    const std::string actual = GetCapturedStdout();
    EXPECT_EQ("expected\n", actual);
}

TEST(console_t, ColoredOutput) {
    console_t sink([](const record_t&) -> color_t {
        return color_t::red();
    });

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    CaptureStdout();
    sink.execute(record, "expected");

    const std::string actual = GetCapturedStdout();
    EXPECT_EQ("\033[31mexpected\033[0m\n", actual);
}

TEST(console_t, FilterAcceptsAll) {
    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    console_t sink;

    EXPECT_TRUE(sink.filter(record));
}

TEST(console_t, Type) {
    EXPECT_EQ("console", std::string(factory<sink::console_t>::type()));
}

}  // namespace sink
}  // namespace testing
}  // namespace blackhole
