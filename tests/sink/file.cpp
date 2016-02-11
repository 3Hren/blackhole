#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <blackhole/detail/sink/file.hpp>

#include "mocks/node.hpp"

namespace blackhole {
namespace testing {
namespace sink {

using ::blackhole::sink::file_t;

using ::testing::Return;
using ::testing::StrictMock;

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

TEST(file_t, FilterAcceptsAll) {
    const string_view message("");
    const attribute_pack pack;
    record_t record(42, message, pack);

    file_t sink("");

    EXPECT_TRUE(sink.filter(record));
}

}  // namespace file

TEST(file_t, Type) {
    EXPECT_EQ("file", std::string(blackhole::factory<file_t>::type()));
}

TEST(file_t, FromRequiresFilename) {
    StrictMock<config::testing::mock::node_t> config;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_THROW(factory<sink::file_t>::from(config), std::logic_error);
}

TEST(file_t, PatternFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<config::testing::mock::node_t> config;

    auto n1 = new node_t;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(n1));

    EXPECT_CALL(*n1, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    const auto file = factory<sink::file_t>::from(config);

    EXPECT_EQ("/tmp/blackhole.log", file.path());
}

}  // namespace sink
}  // namespace testing
}  // namespace blackhole
