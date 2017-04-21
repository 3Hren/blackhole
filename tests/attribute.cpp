#include <gtest/gtest.h>

#include <boost/variant/apply_visitor.hpp>

#include <blackhole/attribute.hpp>
#include "blackhole/extensions/writer.hpp"

#include <src/attribute.hpp>

namespace {

struct user_t {
    std::string name;
};

} // namespace

namespace blackhole {
inline namespace v1 {

template<>
struct display_traits<user_t> {
    static auto apply(const user_t& user, writer_t& wr) -> void {
        wr.write("user_t(name: {})", user.name);
    }
};

} // namespace v1
} // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace attribute {
namespace {

TEST(view_t, Default) {
    view_t v;

    EXPECT_EQ(nullptr, blackhole::attribute::get<std::nullptr_t>(v));
}

TEST(view_t, FromNullptr) {
    view_t v(nullptr);

    EXPECT_EQ(nullptr, blackhole::attribute::get<std::nullptr_t>(v));
}

TEST(view_t, FromBool) {
    view_t v(true);

    EXPECT_EQ(true, blackhole::attribute::get<bool>(v));
}

TEST(view_t, FromChar) {
    char value = 42;
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::int64_t>(v));
}

TEST(view_t, FromShort) {
    short value = 42;
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::int64_t>(v));
}

TEST(view_t, FromInt) {
    view_t v(42);

    EXPECT_EQ(42, blackhole::attribute::get<std::int64_t>(v));
}

TEST(view_t, FromLong) {
    view_t v(42L);

    EXPECT_EQ(42L, blackhole::attribute::get<std::int64_t>(v));
}

TEST(view_t, FromLongLong) {
    view_t v(42LL);

    EXPECT_EQ(42LL, blackhole::attribute::get<std::int64_t>(v));
}

TEST(view_t, FromUnsignedChar) {
    unsigned char value = 42;
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(view_t, FromUnsignedShort) {
    unsigned short value = 42;
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(view_t, FromUnsignedInt) {
    unsigned int value = 42;
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(view_t, FromUnsignedLong) {
    view_t v(42UL);

    EXPECT_EQ(42UL, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(view_t, FromUnsignedLongLong) {
    view_t v(42ULL);

    EXPECT_EQ(42ULL, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(view_t, FromFloat) {
    view_t v(42.5f);

    EXPECT_FLOAT_EQ(42.5f, blackhole::attribute::get<double>(v));
}

TEST(view_t, FromDouble) {
    view_t v(42.5);

    EXPECT_FLOAT_EQ(42.5, blackhole::attribute::get<double>(v));
}

TEST(view_t, FromStringLiteral) {
    view_t v("le message");

    EXPECT_EQ("le message", blackhole::attribute::get<string_view>(v).to_string());
}

TEST(view_t, FromString) {
    std::string value("le message");
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<string_view>(v).to_string());
}

TEST(view_t, FromStringView) {
    string_view value("le message");
    view_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<string_view>(v).to_string());
}

TEST(view_t, FromUserType) {
    user_t user{"Ivan"};
    view_t v(user);

    writer_t wr;
    blackhole::attribute::get<view_t::function_type>(v)(wr);

    EXPECT_EQ("user_t(name: Ivan)", wr.result().to_string());
}

namespace {

struct visitor_t : public boost::static_visitor<bool> {
    auto operator()(std::nullptr_t) const -> bool {
        return true;
    }

    template<typename T>
    auto operator()(const T&) const -> bool {
        return false;
    }
};

}  // namespace

TEST(view_t, DefaultVisit) {
    view_t v;

    EXPECT_TRUE(boost::apply_visitor(visitor_t(), v.inner().value));
}

struct view_visitor : public view_t::visitor_t {
    int value;

    view_visitor() : value(0) {}

    auto operator()(const view_t::null_type&) -> void {}
    auto operator()(const view_t::bool_type&) -> void {}
    auto operator()(const view_t::sint64_type& value) -> void {
        this->value = value;
    }
    auto operator()(const view_t::uint64_type&) -> void {}
    auto operator()(const view_t::double_type&) -> void {}
    auto operator()(const view_t::string_type&) -> void {}
    auto operator()(const view_t::function_type&) -> void {}
};

TEST(view_t, Visitor) {
    view_t v(42);

    view_visitor visitor;

    EXPECT_EQ(0, visitor.value);

    v.apply(visitor);

    EXPECT_EQ(42, visitor.value);
}

TEST(value_t, Default) {
    value_t v;

    EXPECT_EQ(nullptr, blackhole::attribute::get<std::nullptr_t>(v));
}

TEST(value_t, FromNullptr) {
    value_t v(nullptr);

    EXPECT_EQ(nullptr, blackhole::attribute::get<std::nullptr_t>(v));
}

TEST(value_t, FromBool) {
    value_t v(true);

    EXPECT_EQ(true, blackhole::attribute::get<bool>(v));
}

TEST(value_t, FromChar) {
    char value = 42;
    value_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::int64_t>(v));
}

TEST(value_t, FromShort) {
    short value = 42;
    value_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::int64_t>(v));
}

TEST(value_t, FromInt) {
    value_t v(42);

    EXPECT_EQ(42, blackhole::attribute::get<std::int64_t>(v));
}

TEST(value_t, FromLong) {
    value_t v(42L);

    EXPECT_EQ(42L, blackhole::attribute::get<std::int64_t>(v));
}

TEST(value_t, FromLongLong) {
    value_t v(42LL);

    EXPECT_EQ(42LL, blackhole::attribute::get<std::int64_t>(v));
}

TEST(value_t, FromUnsignedChar) {
    unsigned char value = 42;
    value_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(value_t, FromUnsignedShort) {
    unsigned short value = 42;
    value_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(value_t, FromUnsignedInt) {
    unsigned int value = 42;
    value_t v(value);

    EXPECT_EQ(value, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(value_t, FromUnsignedLong) {
    value_t v(42UL);

    EXPECT_EQ(42UL, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(value_t, FromUnsignedLongLong) {
    value_t v(42ULL);

    EXPECT_EQ(42ULL, blackhole::attribute::get<std::uint64_t>(v));
}

TEST(value_t, FromString) {
    value_t v("le message");

    EXPECT_EQ("le message", blackhole::attribute::get<std::string>(v));
}

struct value_visitor : public value_t::visitor_t {
    int value;

    value_visitor() : value(0) {}

    auto operator()(const value_t::null_type&) -> void {}
    auto operator()(const value_t::bool_type&) -> void {}
    auto operator()(const value_t::sint64_type& value) -> void {
        this->value = value;
    }
    auto operator()(const value_t::uint64_type&) -> void {}
    auto operator()(const value_t::double_type&) -> void {}
    auto operator()(const value_t::string_type&) -> void {}
    auto operator()(const value_t::function_type&) -> void {}
};

TEST(value_t, Visitor) {
    value_t v(42);

    value_visitor visitor;

    EXPECT_EQ(0, visitor.value);

    v.apply(visitor);

    EXPECT_EQ(42, visitor.value);
}

} // namespace
} // namespace attribute
} // namespace v1
} // namespace blackhole
