#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>

namespace blackhole {
namespace testing {
namespace attribute {

using ::blackhole::attribute::view_t;

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

}  // namespace attribute
}  // namespace testing
}  // namespace blackhole
