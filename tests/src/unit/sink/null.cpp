#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/sink.hpp>
#include <blackhole/sink/null.hpp>

#include "mocks/node.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using ::testing::StrictMock;

using experimental::factory;

TEST(null_t, type) {
    EXPECT_EQ(std::string("null"), factory<sink::null_t>().type());
}

TEST(null_t, factory) {
    StrictMock<config::testing::mock::node_t> config;

    // NOTE: Actually does nothing, none of mock methods should be called.
    factory<sink::null_t>().from(config);
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
