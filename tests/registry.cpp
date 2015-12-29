#include <gtest/gtest.h>

#include <blackhole/registry.hpp>
#include <blackhole/root.hpp>

#include "mocks/node.hpp"

namespace blackhole {
namespace testing {

using ::testing::ByRef;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

TEST(registry_t, ThrowsOnAbsentFormatter) {
    using config::testing::mock::node_t;

    registry_t registry;

    node_t n0;
    auto n1 = new node_t;
    node_t n2;

    config::factory<node_t> factory;

    try {
        auto builder = registry.builder<node_t>();

        auto& factory = dynamic_cast<config::factory<node_t>&>(builder.configurator());
        EXPECT_CALL(factory, config())
            .Times(1)
            .WillOnce(ReturnRef(n0));

        EXPECT_CALL(n0, subscript_key("root"))
            .Times(1)
            .WillOnce(Return(n1));

        EXPECT_CALL(*n1, each(_))
            .Times(1)
            .WillOnce(Invoke([&](const node_t::each_function& fn) {
                fn(n2);
            }));

        EXPECT_CALL(n2, subscript_key("formatter"))
            .Times(1)
            .WillOnce(Return(nullptr));

        builder.build("root");
        FAIL();
    } catch (const std::logic_error& err) {
        EXPECT_STREQ("each handler must have a formatter", err.what());
    }
}

}  // namespace testing
}  // namespace blackhole
