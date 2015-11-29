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

using ::blackhole::sink::console_t;

TEST(console_t, ByDefaultUseStandardOutput) {
    console_t sink;

    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    CaptureStdout();
    sink.execute(record, "expected");

    const std::string actual = GetCapturedStdout();
    EXPECT_EQ("expected\n", actual);
}

TEST(console_t, AcceptsAll) {
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
