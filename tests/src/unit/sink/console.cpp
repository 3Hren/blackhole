#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/console.hpp>

#include <src/sink/console.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using ::testing::StrictMock;
using ::testing::internal::CaptureStdout;
using ::testing::internal::GetCapturedStdout;

using experimental::color_t;
using experimental::factory;

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
    sink.emit(record, "expected");

    const std::string actual = GetCapturedStdout();
    EXPECT_EQ("expected\n", actual);
}

class cout_redirector_t {
    std::streambuf* prev;

public:
    cout_redirector_t(std::streambuf* buffer) :
        prev(std::cout.rdbuf(buffer))
    {}

    ~cout_redirector_t() {
        std::cout.rdbuf(prev);
    }
};

TEST(console_t, ColoredOutput) {
    console_t sink(std::cout, [](const record_t&) -> color_t {
        return color_t::red();
    });

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    std::ostringstream stream;
    cout_redirector_t lock(stream.rdbuf());
    sink.emit(record, "expected");

    if (::isatty(1)) {
        EXPECT_EQ("\033[31mexpected\033[0m\n", stream.str());
    } else {
        EXPECT_EQ("expected\n", stream.str());
    }
}

TEST(console_t, NonColoredOutputToNonTTY) {
    std::stringstream stream;
    console_t sink(stream, [](const record_t&) -> color_t {
        return color_t::red();
    });

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    sink.emit(record, "expected");

    EXPECT_EQ("expected\n", stream.str());
}

TEST(console_t, FactoryType) {
    EXPECT_EQ(std::string("console"), factory<sink::console_t>().type());
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
