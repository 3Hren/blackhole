#include <gtest/gtest.h>

#include <boost/variant/apply_visitor.hpp>

#include <blackhole/attribute.hpp>

#include <blackhole/detail/attribute.hpp>

namespace blackhole {
namespace testing {
namespace attribute {

using ::blackhole::attribute::value_t;
using ::blackhole::attribute::view_t;

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

TEST(value_t, Default) {
    value_t v;

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

struct visitor_t : public boost::static_visitor<bool> {
    auto operator()(std::nullptr_t) const -> bool {
        return true;
    }

    template<typename T>
    auto operator()(const T&) const -> bool {
        return false;
    }
};

TEST(view_t, DefaultVisit) {
    view_t v;

    EXPECT_TRUE(boost::apply_visitor(visitor_t(), v.inner().value));
}

}  // namespace attribute
}  // namespace testing
}  // namespace blackhole
