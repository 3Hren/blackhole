#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <blackhole/detail/sink/file.hpp>

namespace blackhole {
namespace testing {
namespace sink {

using ::blackhole::sink::file_t;

namespace file {

using ::blackhole::sink::file::inner_t;

TEST(inner_t, IntervalSanitizer) {
    inner_t inner("", 0);

    EXPECT_NE(0, inner.interval());
    EXPECT_EQ(std::numeric_limits<std::size_t>::max(), inner.interval());
}

TEST(inner_t, IntervalOverflow) {
    std::size_t interval = 3;
    std::size_t counter = 0;

    counter = (counter + 1) % interval;
    EXPECT_EQ(1, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(2, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(0, counter);
}

}  // namespace file

TEST(file_t, FilterAcceptsAll) {
    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    file_t sink("");

    EXPECT_TRUE(sink.filter(record));
}

TEST(file_t, type) {
    EXPECT_EQ("file", std::string(blackhole::factory<file_t>::type()));
}

}  // namespace sink
}  // namespace testing
}  // namespace blackhole
