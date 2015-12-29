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

// TEST(registry_t, ThrowsOnUnregisteredHandler) {
// EXPECT_STREQ(R"(handler with type "unknown" isn't registered)", err.what());

// + нет форматтера.
// - тип не указан для форматтера
// - форматтер заданного типа не зарегистрирован
// - тип не указан для синка
// - синк заданного типа не зарегистрирован
// - тип не указан для хендлера - блокинг по-умолчанию
// - хендлер заданного типа не зарегистрирован
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

TEST(registry_t, ThrowsOnFormatterWithNoType) {
    using config::testing::mock::node_t;

    registry_t registry;

    node_t n0;
    auto n1 = new node_t;
    node_t n2;
    auto n3 = new node_t;

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
            .WillOnce(Return(n3));

        EXPECT_CALL(*n3, subscript_key("type"))
            .Times(1)
            .WillOnce(Return(nullptr));

        builder.build("root");
        FAIL();
    } catch (const std::logic_error& err) {
        EXPECT_STREQ("formatter must have a type", err.what());
    }
}

}  // namespace testing
}  // namespace blackhole
