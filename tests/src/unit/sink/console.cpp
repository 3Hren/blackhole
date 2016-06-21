#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/filter.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/console.hpp>
#include <blackhole/termcolor.hpp>

#include <src/sink/console.hpp>

#include "mocks/registry.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using ::testing::StrictMock;
using ::testing::internal::CaptureStdout;
using ::testing::internal::GetCapturedStdout;

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
    console_t sink(std::cout, [](const record_t&) -> termcolor_t {
        return termcolor_t::red();
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
    console_t sink(stream, [](const record_t&) -> termcolor_t {
        return termcolor_t::red();
    });

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    sink.emit(record, "expected");

    EXPECT_EQ("expected\n", stream.str());
}

TEST(console_t, FactoryType) {
    EXPECT_EQ(std::string("console"), factory<sink::console_t>(mock_registry_t()).type());
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
