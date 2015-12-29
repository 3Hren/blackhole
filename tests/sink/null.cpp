#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/config/node.hpp>
#include <blackhole/config/option.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/null.hpp>

#include "mocks/node.hpp"

namespace blackhole {
namespace testing {

using ::testing::StrictMock;

using sink::null_t;

TEST(null_t, FilterOut) {
    const string_view message("-");
    const attribute_pack pack;
    record_t record(42, message, pack);

    null_t sink;

    EXPECT_FALSE(sink.filter(record));
}

TEST(null_t, type) {
    EXPECT_EQ("null", std::string(factory<sink::null_t>::type()));
}

TEST(null_t, factory) {
    StrictMock<config::testing::mock::node_t> config;

    // NOTE: Actually does nothing, none of mock methods should be called.
    factory<sink::null_t>::from(config);
}

}  // namespace testing
}  // namespace blackhole
