#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <blackhole/detail/sink/file.hpp>

namespace blackhole {
namespace testing {

using ::testing::StrictMock;

using ::blackhole::sink::file_t;

TEST(console_t, FilterAcceptsAll) {
    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    file_t sink("");

    EXPECT_TRUE(sink.filter(record));
}

TEST(file_t, type) {
    EXPECT_EQ("file", std::string(factory<sink::file_t>::type()));
}

}  // namespace testing
}  // namespace blackhole
