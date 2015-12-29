#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/optional/optional.hpp>

#include <blackhole/config/node.hpp>
#include <blackhole/config/option.hpp>

#include "mocks/node.hpp"

namespace blackhole {
namespace config {
namespace testing {

using ::testing::Invoke;
using ::testing::Return;
using ::testing::_;

TEST(option, None) {
    option<node_t> root;

    EXPECT_TRUE(!root);
}

TEST(option, Some) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_TRUE(!!root);
}

TEST(option, Unwrap) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_EQ(node, &root.unwrap().get());
}

TEST(option, GetBoolNone) {
    option<node_t> root;

    ASSERT_TRUE(!root.to_bool());
}

TEST(option, GetBool) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_CALL(*node, to_bool())
        .Times(1)
        .WillOnce(Return(true));

    const auto actual = root.to_bool();

    ASSERT_TRUE(!!actual);
    EXPECT_EQ(true, actual.get());
}

TEST(option, GetIntNone) {
    option<node_t> root;

    ASSERT_TRUE(!root.to_sint64());
}

TEST(option, GetInt) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_CALL(*node, to_sint64())
        .Times(1)
        .WillOnce(Return(42));

    const auto actual = root.to_sint64();

    ASSERT_TRUE(!!actual);
    EXPECT_EQ(42, actual.get());
}

TEST(option, GetUIntNone) {
    option<node_t> root;

    ASSERT_TRUE(!root.to_uint64());
}

TEST(option, GetUInt) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_CALL(*node, to_uint64())
        .Times(1)
        .WillOnce(Return(42));

    const auto actual = root.to_uint64();

    ASSERT_TRUE(!!actual);
    EXPECT_EQ(42, actual.get());
}

TEST(option, GetDoubleNone) {
    option<node_t> root;

    ASSERT_TRUE(!root.to_double());
}

TEST(option, GetDouble) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_CALL(*node, to_double())
        .Times(1)
        .WillOnce(Return(3.1415));

    const auto actual = root.to_double();

    ASSERT_TRUE(!!actual);
    EXPECT_DOUBLE_EQ(3.1415, actual.get());
}

TEST(option, GetStringNone) {
    option<node_t> root;

    ASSERT_TRUE(!root.to_string());
}

TEST(option, GetString) {
    auto node = new mock::node_t;

    option<node_t> root((std::unique_ptr<node_t>(node)));

    EXPECT_CALL(*node, to_string())
        .Times(1)
        .WillOnce(Return("value"));

    const auto actual = root.to_string();

    ASSERT_TRUE(!!actual);
    EXPECT_EQ("value", actual.get());
}

TEST(option, EachNone) {
    option<node_t> root;

    int counter = 0;

    root.each([&](const node_t&) {
        ++counter;
    });

    EXPECT_EQ(0, counter);
}

TEST(option, Each) {
    auto outer = new mock::node_t;
    mock::node_t inner;

    option<node_t> root((std::unique_ptr<node_t>(outer)));

    EXPECT_CALL(*outer, each(_))
        .Times(1)
        .WillOnce(Invoke([&](const node_t::each_function& fn) {
            fn(inner);
        }));

    EXPECT_CALL(inner, to_string())
        .Times(1)
        .WillOnce(Return("value"));

    int counter = 0;

    root.each([&](const node_t& node) {
        ++counter;
        EXPECT_EQ("value", node.to_string());
    });

    EXPECT_EQ(1, counter);
}

TEST(option, EachMapNone) {
    option<node_t> root;

    int counter = 0;

    root.each_map([&](const std::string&, const node_t&) {
        ++counter;
    });

    EXPECT_EQ(0, counter);
}

TEST(option, EachMap) {
    auto outer = new mock::node_t;
    mock::node_t inner;

    option<node_t> root((std::unique_ptr<node_t>(outer)));

    EXPECT_CALL(*outer, each_map(_))
        .Times(1)
        .WillOnce(Invoke([&](const node_t::member_function& fn) {
            fn("name", inner);
        }));

    EXPECT_CALL(inner, to_string())
        .Times(1)
        .WillOnce(Return("value"));

    int counter = 0;

    root.each_map([&](const std::string& name, const node_t& node) {
        ++counter;
        EXPECT_EQ("name", name);
        EXPECT_EQ("value", node.to_string());
    });

    EXPECT_EQ(1, counter);
}

TEST(option, Index) {
    auto n1 = new mock::node_t;
    auto n2 = new mock::node_t;

    option<node_t> o1((std::unique_ptr<node_t>(n1)));

    EXPECT_CALL(*n1, subscript_idx(0))
        .Times(1)
        .WillOnce(Return(n2));

    EXPECT_CALL(*n2, to_string())
        .Times(1)
        .WillOnce(Return("value"));

    const auto actual = o1[0].to_string();

    ASSERT_TRUE(!!actual);
    EXPECT_EQ("value", actual.get());
}

TEST(option, IndexKey) {
    auto n1 = new mock::node_t;
    auto n2 = new mock::node_t;

    option<node_t> o1((std::unique_ptr<node_t>(n1)));

    EXPECT_CALL(*n1, subscript_key("k1"))
        .Times(1)
        .WillOnce(Return(n2));

    EXPECT_CALL(*n2, to_string())
        .Times(1)
        .WillOnce(Return("value"));

    const auto actual = o1["k1"].to_string();

    ASSERT_TRUE(!!actual);
    EXPECT_EQ("value", actual.get());
}

}  // namespace testing
}  // namespace config
}  // namespace blackhole
