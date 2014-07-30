#include "global.hpp"

#include "blackhole/dynamic.hpp"

using namespace blackhole;

TEST(Dynamic, Class) {
    dynamic_t d;
    UNUSED(d)
}

TEST(Dynamic, CopyConstructible) {
    dynamic_t d(42);
    dynamic_t copy(d);
    EXPECT_EQ(42, d.to<int>());
    EXPECT_EQ(42, copy.to<int>());
}

TEST(Dynamic, CopyAssignable) {
    dynamic_t d(42);
    dynamic_t copy;
    copy = d;
    EXPECT_EQ(42, d.to<int>());
    EXPECT_EQ(42, copy.to<int>());
}

TEST(Dynamic, MoveConstructible) {
    dynamic_t d(42);
    dynamic_t moved(std::move(d));
    EXPECT_TRUE(d.invalid());
    EXPECT_EQ(42, moved.to<int>());
}

TEST(Dynamic, MoveAssignable) {
    dynamic_t d(42);
    dynamic_t moved;
    moved = std::move(d);
    EXPECT_TRUE(d.invalid());
    EXPECT_EQ(42, moved.to<int>());
}

TEST(Dynamic, ThrowsExceptionWhenPrecisionLossOccurs) {
    EXPECT_THROW(dynamic_t(256u).to<unsigned char>(), dynamic::positive_overflow);
    EXPECT_THROW(dynamic_t(256).to<unsigned char>(), dynamic::positive_overflow);
}

TEST(Dynamic, ThrowsExceptionWhenUnderflowOccurs) {
    EXPECT_THROW(dynamic_t(-1).to<unsigned char>(), dynamic::negative_overflow);
}

TEST(Dynamic, ThrowsExceptionWhenOverflowOccurs) {
    std::uint64_t value = std::numeric_limits<std::int64_t>::max() + 1;
    EXPECT_THROW(dynamic_t(value).to<
                    std::int64_t
                 >(),
                 dynamic::positive_overflow);
}

TEST(Dynamic, ThrowsExceptionOnInvalidCast) {
    EXPECT_THROW(dynamic_t(42.5).to<unsigned char>(), dynamic::bad_cast);
    EXPECT_THROW(dynamic_t("le shit").to<bool>(), dynamic::bad_cast);
}

TEST(Dynamic, ThrowsExceptionOnRequestingIndexFromNonArray) {
    EXPECT_THROW(dynamic_t(42.5)[1], dynamic::bad_cast);

    const dynamic_t d(42.5);
    EXPECT_THROW(d[1], dynamic::bad_cast);
}

TEST(Dynamic, ThrowsExceptionOnRequestingIndexFromNonObject) {
    EXPECT_THROW(dynamic_t(42.5)["key"], dynamic::bad_cast);

    const dynamic_t d(42.5);
    EXPECT_THROW(d["key"], dynamic::bad_cast);
}

TEST(Dynamic, IsNullByDefault) {
    dynamic_t d;

    EXPECT_TRUE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsBool) {
    dynamic_t d = true;

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_TRUE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsUint) {
    dynamic_t d = 42u;

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_TRUE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsInt) {
    dynamic_t d = 42;

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_TRUE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsDouble) {
    dynamic_t d = 42.5;

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_TRUE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsString) {
    dynamic_t d = "le govno";

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_TRUE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsArray) {
    dynamic_t d;
    d[0] = "item";

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_TRUE(d.is<dynamic_t::array_t>());
    EXPECT_FALSE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, IsObject) {
    dynamic_t d;
    d["key"] = "value";

    EXPECT_FALSE(d.is<dynamic_t::null_t>());
    EXPECT_FALSE(d.is<dynamic_t::bool_t>());
    EXPECT_FALSE(d.is<dynamic_t::uint_t>());
    EXPECT_FALSE(d.is<dynamic_t::int_t>());
    EXPECT_FALSE(d.is<dynamic_t::double_t>());
    EXPECT_FALSE(d.is<dynamic_t::string_t>());
    EXPECT_FALSE(d.is<dynamic_t::array_t>());
    EXPECT_TRUE(d.is<dynamic_t::object_t>());
}

TEST(Dynamic, LikeDictBool) {
    dynamic_t base;
    base["input"] = true;
    base["output"] = false;

    EXPECT_EQ(true, base["input"].to<bool>());
    EXPECT_EQ(false, base["output"].to<bool>());
}

TEST(Dynamic, LikeDictUint8) {
    unsigned char actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned char>());
}

TEST(Dynamic, LikeDictUint16) {
    unsigned short actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned short>());
}

TEST(Dynamic, LikeDictUint32) {
    unsigned int actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned int>());
}

TEST(Dynamic, LikeDictUint64) {
    unsigned long long actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<unsigned long long>());
}

TEST(Dynamic, LikeDictInt8) {
    char actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<char>());
}

TEST(Dynamic, LikeDictInt16) {
    short actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<short>());
}

TEST(Dynamic, LikeDictInt32) {
    int actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<int>());
}

TEST(Dynamic, LikeDictInt64) {
    long long actual = 42;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<long long>());
}

TEST(Dynamic, LikeDictDouble) {
    double actual = 42.100500;

    dynamic_t base;
    base["output"] = actual;

    EXPECT_EQ(actual, base["output"].to<double>());
}

TEST(Dynamic, LikeDictString) {
    dynamic_t base;
    base["output"] = "stdout";

    EXPECT_EQ("stdout", base["output"].to<std::string>());
}

TEST(Dynamic, LikeDictArray) {
    dynamic_t::array_t array;
    array.push_back(42);

    dynamic_t base;
    base["output"] = array;

    EXPECT_EQ(array, base["output"].to<dynamic_t::array_t>());
}

TEST(Dynamic, LikeDictMap) {
    dynamic_t::object_t map;
    map["key"] = 42;

    dynamic_t base;
    base["output"] = map;

    EXPECT_EQ(map, base["output"].to<dynamic_t::object_t>());
}

TEST(Dynamic, NestedMap) {
    dynamic_t base;
    base["output"]["verbose"] = true;

    EXPECT_EQ(true, base["output"]["verbose"].to<bool>());
}

TEST(Dynamic, NestedArray) {
    dynamic_t base;
    base["output"][1] = true;

    EXPECT_EQ(true, base["output"][1].to<bool>());
}

TEST(Dynamic, ContainsIfObject) {
    dynamic_t d;
    d["key"] = "value";
    EXPECT_TRUE(d.contains("key"));
}

TEST(Dynamic, NotContainsIfObject) {
    dynamic_t d;
    d["key"] = "value";
    EXPECT_FALSE(d.contains("key-value"));
}
