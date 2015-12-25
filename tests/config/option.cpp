#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

// blackhole/config/node.hpp

#include <functional>

namespace blackhole {
namespace config {

template<typename T>
class option;

/// Config node.
class node_t {
public:
    typedef std::function<auto(const node_t& node) -> void> each_function;
    typedef std::function<auto(const std::string& key, const node_t& node) -> void> member_function;

public:
    virtual ~node_t() = 0;

    virtual auto to_bool() const -> bool = 0;
    virtual auto to_sint64() const -> std::int64_t = 0;
    virtual auto to_uint64() const -> std::uint64_t = 0;
    virtual auto to_double() const -> double = 0;
    virtual auto to_string() const -> std::string = 0;

    virtual auto each(const each_function& fn) -> void = 0;
    virtual auto each_map(const member_function& fn) -> void = 0;

    virtual auto operator[](const std::size_t& idx) const -> option<node_t> = 0;
    virtual auto operator[](const std::string& key) const -> option<node_t> = 0;
};

}  // namespace config
}  // namespace blackhole

// blackhole/config/node.hpp

#include <memory>

#include <boost/optional/optional_fwd.hpp>

namespace blackhole {
namespace config {

template<typename T>
class option;

template<>
class option<node_t> {
    std::unique_ptr<node_t> node;

public:
    /// Constructs an option object that will contan nothing.
    option() noexcept;

    /// Constructs an option object that will contain the specified configuration node.
    explicit option(std::unique_ptr<node_t> node) noexcept;

    auto to_bool() const -> boost::optional<bool>;
    auto to_sint64() const -> boost::optional<std::int64_t>;
    auto to_uint64() const -> boost::optional<std::uint64_t>;
    auto to_double() const -> boost::optional<double>;
    auto to_string() const -> boost::optional<std::string>;

    auto each(const node_t::each_function& fn) -> void;
    auto each_map(const node_t::member_function& fn) -> void;

    auto operator[](const std::size_t& idx) const -> option<node_t>;
    auto operator[](const std::string& key) const -> option<node_t>;

private:
    template<typename F>
    auto to(F&& fn) const -> decltype(fn());
};

/// Constructs an option of the specified configuration node type using given arguments.
template<typename T, typename... Args>
auto make_option(Args&&... args) -> option<node_t> {
    return option<node_t>(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}  // namespace config
}  // namespace blackhole

// blackhole/config/node.cpp

namespace blackhole {
namespace config {

node_t::~node_t() = default;

}  // namespace config
}  // namespace blackhole

// blackhole/config/config.cpp

#include <boost/optional/optional.hpp>

namespace blackhole {
namespace config {

option<node_t>::option() noexcept = default;
option<node_t>::option(std::unique_ptr<node_t> node) noexcept :
    node(std::move(node))
{}

auto option<node_t>::to_bool() const -> boost::optional<bool> {
    return to([&]() -> boost::optional<bool> {
        return node->to_bool();
    });
}

auto option<node_t>::to_sint64() const -> boost::optional<std::int64_t> {
    return to([&]() -> boost::optional<std::int64_t> {
        return node->to_sint64();
    });
}

auto option<node_t>::to_uint64() const -> boost::optional<std::uint64_t> {
    return to([&]() -> boost::optional<std::uint64_t> {
        return node->to_uint64();
    });
}

auto option<node_t>::to_double() const -> boost::optional<double> {
    return to([&]() -> boost::optional<double> {
        return node->to_double();
    });
}

auto option<node_t>::to_string() const -> boost::optional<std::string> {
    return to([&]() -> boost::optional<std::string> {
        return node->to_string();
    });
}

auto option<node_t>::each(const node_t::each_function& fn) -> void {
    if (node) {
        node->each(fn);
    }
}

auto option<node_t>::each_map(const node_t::member_function& fn) -> void {
    if (node) {
        node->each_map(fn);
    }
}

auto option<node_t>::operator[](const std::size_t& idx) const -> option<node_t> {
    return to([&]() -> option<node_t> {
        return (*node)[idx];
    });
}

auto option<node_t>::operator[](const std::string& key) const -> option<node_t> {
    return to([&]() -> option<node_t> {
        return (*node)[key];
    });
}

template<typename F>
auto option<node_t>::to(F&& fn) const -> decltype(fn()) {
    if (node) {
        return fn();
    } else {
        return {};
    }
}

}  // namespace config
}  // namespace blackhole

namespace blackhole {
namespace config {
namespace testing {
namespace mock {

class node_t : public config::node_t {
public:
    MOCK_CONST_METHOD0(to_bool, bool());
    MOCK_CONST_METHOD0(to_sint64, std::int64_t());
    MOCK_CONST_METHOD0(to_uint64, std::uint64_t());
    MOCK_CONST_METHOD0(to_double, double());
    MOCK_CONST_METHOD0(to_string, std::string());

    MOCK_METHOD1(each, void(const each_function&));
    MOCK_METHOD1(each_map, void(const member_function&));

    MOCK_CONST_METHOD1(subscript_idx, config::node_t*(const std::size_t&));
    MOCK_CONST_METHOD1(subscript_key, config::node_t*(const std::string&));

    auto operator[](const std::size_t& idx) const -> option<config::node_t> {
        return option<config::node_t>(std::unique_ptr<config::node_t>(subscript_idx(idx)));
    }

    auto operator[](const std::string& key) const -> option<config::node_t> {
        return option<config::node_t>(std::unique_ptr<config::node_t>(subscript_key(key)));
    }
};

}  // namespace mock
}  // namespace testing
}  // namespace config
}  // namespace blackhole

namespace blackhole {
namespace config {
namespace testing {

using ::testing::Invoke;
using ::testing::Return;
using ::testing::_;

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
    EXPECT_EQ(true, actual.value());
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
    EXPECT_EQ(42, actual.value());
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
    EXPECT_EQ(42, actual.value());
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
    EXPECT_DOUBLE_EQ(3.1415, actual.value());
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
    EXPECT_EQ("value", actual.value());
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
    EXPECT_EQ("value", actual.value());
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
    EXPECT_EQ("value", actual.value());
}

}  // namespace testing
}  // namespace config
}  // namespace blackhole
