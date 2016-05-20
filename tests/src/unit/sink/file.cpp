#include <system_error>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/file.hpp>
#include <blackhole/detail/sink/file.hpp>

#include "mocks/node.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace {

using experimental::factory;

using ::testing::Return;
using ::testing::StrictMock;

TEST(file_t, IntervalSanitizer) {
    file_t sink("", 0);

    EXPECT_NE(0, sink.interval());
    EXPECT_EQ(std::numeric_limits<std::size_t>::max(), sink.interval());
}

TEST(file_t, IntervalOverflow) {
    std::size_t interval = 3;
    std::size_t counter = 0;

    counter = (counter + 1) % interval;
    EXPECT_EQ(1, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(2, counter);

    counter = (counter + 1) % interval;
    EXPECT_EQ(0, counter);
}

TEST(factory, Type) {
    EXPECT_EQ(std::string("file"), factory<file_t>().type());
}

TEST(factory, FromRequiresFilename) {
    StrictMock<config::testing::mock::node_t> config;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(nullptr));

    EXPECT_THROW(factory<file_t>().from(config), std::invalid_argument);
}

TEST(factory, PatternFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto npath = new node_t;

    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nullptr));

    auto sink = factory<file_t>().from(config);
    const auto& cast = dynamic_cast<const file_t&>(*sink);

    EXPECT_EQ("/tmp/blackhole.log", cast.path());
}

TEST(factory, FlushIntervalFromConfig) {
    using config::testing::mock::node_t;

    StrictMock<node_t> config;

    auto npath = new node_t;
    EXPECT_CALL(config, subscript_key("path"))
        .Times(1)
        .WillOnce(Return(npath));

    EXPECT_CALL(*npath, to_string())
        .Times(1)
        .WillOnce(Return("/tmp/blackhole.log"));

    auto nflush = new node_t;
    EXPECT_CALL(config, subscript_key("flush"))
        .Times(1)
        .WillOnce(Return(nflush));

    EXPECT_CALL(*nflush, to_uint64())
        .Times(1)
        .WillOnce(Return(30));

    factory<file_t>().from(config);
}

}  // namespace
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
