#include <type_traits>

#include <blackhole/attribute.hpp>
#include <blackhole/detail/traits/same.hpp>
#include <blackhole/log.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(Traits, IsSupportedAttributeType) {
    static_assert(log::attribute::is_supported<std::uint8_t>::value,    "`std::uint8_t` must be supported");
    static_assert(log::attribute::is_supported<std::uint32_t>::value,   "`std::uint32_t` must be supported");
    static_assert(log::attribute::is_supported<std::int32_t>::value,    "`std::int32_t` must be supported");
    static_assert(log::attribute::is_supported<std::uint64_t>::value,   "`std::uint64_t` must be supported");
    static_assert(log::attribute::is_supported<std::int64_t>::value,    "`std::int64_t` must be supported");
    static_assert(log::attribute::is_supported<std::double_t>::value,   "`std::double_t` must be supported");
    static_assert(log::attribute::is_supported<std::string>::value,     "`std::string` must be supported");
    static_assert(log::attribute::is_supported<timeval>::value,         "`timeval` must be supported");
}

TEST(Traits, LiteralIsConvertibleToAttribute) {
    static_assert(log::attribute::is_constructible<const char*>::value,
                  "`const char*` must be convertible");
}

TEST(Traits, StringIsConvertibleToAttribute) {
    static_assert(log::attribute::is_constructible<const std::string&>::value,
                  "`const std::string&` must be convertible");
}

TEST(Traits, AnyCharIsImplicitConvertible) {
    static_assert(log::attribute::is_constructible<char>::value,
                  "`char` must be convertible");
    log::attribute_value_t(static_cast<char>(42));

    static_assert(log::attribute::is_constructible<unsigned char>::value,
                  "`unsigned char` must be convertible");
    log::attribute_value_t(static_cast<unsigned char>(42));

    static_assert(log::attribute::is_constructible<const char>::value,
                  "`const char` must be convertible");
    log::attribute_value_t(static_cast<const char>(42));

    static_assert(log::attribute::is_constructible<const unsigned char>::value,
                  "`const unsigned char` must be convertible");
    log::attribute_value_t(static_cast<const unsigned char>(42));
}

TEST(Traits, AnyShortIsImplicitConvertible) {
    static_assert(log::attribute::is_constructible<short>::value,
                  "`short` must be convertible");
    log::attribute_value_t(static_cast<short>(42));

    static_assert(log::attribute::is_constructible<unsigned short>::value,
                  "`unsigned short` must be convertible");
    log::attribute_value_t(static_cast<unsigned short>(42));

    static_assert(log::attribute::is_constructible<const short>::value,
                  "`const short` must be convertible");
    log::attribute_value_t(static_cast<const short>(42));

    static_assert(log::attribute::is_constructible<const unsigned short>::value,
                  "`const unsigned short` must be convertible");
    log::attribute_value_t(static_cast<const unsigned short>(42));
}

TEST(Traits, AnyIntIsImplicitConvertible) {
    static_assert(log::attribute::is_constructible<int>::value,
                  "`int` must be convertible");
    log::attribute_value_t(static_cast<int>(42));

    static_assert(log::attribute::is_constructible<unsigned int>::value,
                  "`unsigned int` must be convertible");
    log::attribute_value_t(static_cast<unsigned int>(42));

    static_assert(log::attribute::is_constructible<const int>::value,
                  "`const int` must be convertible");
    log::attribute_value_t(static_cast<const int>(42));

    static_assert(log::attribute::is_constructible<const unsigned int>::value,
                  "`const unsigned int` must be convertible");
    log::attribute_value_t(static_cast<const unsigned int>(42));
}

TEST(Traits, AnyLongLongIsImplicitConvertible) {
    static_assert(log::attribute::is_constructible<long long>::value,
                  "`long long` must be convertible");
    log::attribute_value_t(static_cast<long long>(42));

    static_assert(log::attribute::is_constructible<unsigned long long>::value,
                  "`unsigned long long` must be convertible");
    log::attribute_value_t(static_cast<unsigned long long>(42));

    static_assert(log::attribute::is_constructible<const long long>::value,
                  "`const long long` must be convertible");
    log::attribute_value_t(static_cast<const long long>(42));

    static_assert(log::attribute::is_constructible<const unsigned long long>::value,
                  "`const unsigned long long` must be convertible");
    log::attribute_value_t(static_cast<const unsigned long long>(42));
}

TEST(Traits, AreSame) {
    static_assert(aux::are_same<int>::value, "`are_same` is broken on zero parameters");
    static_assert(aux::are_same<int, int>::value, "`are_same` is broken on single parameter");
    static_assert(aux::are_same<int, int, int>::value, "`are_same` is broken on multiple parameters");
}

TEST(Traits, AddIndexSimpleTuple) {
    static_assert(
        std::is_same<
            tuple::add_index<
                std::tuple<char>
            >::type,
            std::tuple<
                std::tuple<std::integral_constant<int, 0>, char>
            >
        >::value,
        "add_index is broken");
}

TEST(Traits, AddIndexComplexTuple) {
    static_assert(
        std::is_same<
            tuple::add_index<
                std::tuple<char, short, int, double>
            >::type,
            std::tuple<
                std::tuple<std::integral_constant<int, 0>, char>,
                std::tuple<std::integral_constant<int, 1>, short>,
                std::tuple<std::integral_constant<int, 2>, int>,
                std::tuple<std::integral_constant<int, 3>, double>
            >
        >::value,
        "add_index is broken");
}

TEST(Traits, RemoveIndexSimpleTuple) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                std::tuple<
                    std::tuple<std::integral_constant<int, 0>, char>
                >
            >::type,
            std::tuple<char>
        >::value,
        "remove_index is broken");
}

TEST(Traits, OddFilterOddCount) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<1, -1, 2>::type,
                    tuple::add_index<
                        std::tuple<int, float, double>
                    >::type
                >::type
            >::type,
            std::tuple<float>
        >::value,
        "`odd` filter is broken");
}

TEST(Traits, OddFilterEvenCount) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<1, -1, 2>::type,
                    tuple::add_index<
                        std::tuple<int, float, double, short>
                    >::type
                >::type
            >::type,
            std::tuple<float, short>
        >::value,
        "`odd` filter is broken");
}

TEST(Traits, EvenFilterOddCount) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<0, -1, 2>::type,
                    tuple::add_index<
                        std::tuple<int, float, double>
                    >::type
                >::type
            >::type,
            std::tuple<int, double>
        >::value,
        "`even` filter is broken");
}

TEST(Traits, EvenFilterEvenCount) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<0, -1, 2>::type,
                    tuple::add_index<
                        std::tuple<int, float, double, short>
                    >::type
                >::type
            >::type,
            std::tuple<int, double>
        >::value,
        "`even` filter is broken");
}

TEST(Traits, All) {
    static_assert(
        tuple::all<std::tuple<std::true_type, std::true_type>>::value,
        "`all` map is broken");
}

TEST(Traits, IsStringLiteralTypeAuto) {
    auto literal = "blah";
    static_assert(
        is_string_literal_type<decltype(literal)>::type::value,
        "`is_string_literal_type` is broken");
}

TEST(Traits, IsStringLiteralTypeConstCharArray) {
    const char* literal = "blah";
    static_assert(
        is_string_literal_type<decltype(literal)>::type::value,
        "`is_string_literal_type` is broken");
}

TEST(Traits, IsStringLiteralTypeCharArray) {
    char literal[] = "blah";
    static_assert(
        is_string_literal_type<decltype(literal)>::type::value,
        "`is_string_literal_type` is broken");
}

TEST(Traits, SliceStart) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<1>::type,
                    tuple::add_index<
                        std::tuple<char, short, int>
                    >::type
                >::type
            >::type,
            std::tuple<short, int>
        >::value,
        "`slice` filter is broken");
}

TEST(Traits, SliceStop) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<0, 2>::type,
                    tuple::add_index<
                        std::tuple<char, short, int>
                    >::type
                >::type
            >::type,
            std::tuple<char, short>
        >::value,
        "`slice` filter is broken");
}

TEST(Traits, SliceStep) {
    static_assert(
        std::is_same<
            tuple::remove_index<
                tuple::filter<
                    tuple::slice<0, -1, 2>::type,
                    tuple::add_index<
                        std::tuple<char, short, int, double>
                    >::type
                >::type
            >::type,
            std::tuple<char, int>
        >::value,
        "`slice` filter is broken");
}
