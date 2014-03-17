#include "common.hpp"

//! \brief Depends on `tm_year` - years since 1900.

class year_generator_test_case_t : public generator_test_case_t {};

TEST_F(year_generator_test_case_t, FullYear) {
    tm.tm_year = 114;
    EXPECT_EQ("2014", generate("%Y"));
    EXPECT_EQ(common::using_strftime("%Y", tm), generate("%Y"));
}

TEST_F(year_generator_test_case_t, ShortYear) {
    tm.tm_year = 114;
    EXPECT_EQ("14", generate("%y"));
    EXPECT_EQ(common::using_strftime("%y", tm), generate("%y"));
}

TEST_F(year_generator_test_case_t, ShortYearWithZeroPrefix) {
    tm.tm_year = 104;
    EXPECT_EQ("04", generate("%y"));
    EXPECT_EQ(common::using_strftime("%y", tm), generate("%y"));
}

TEST_F(year_generator_test_case_t, ShortYearMinValue) {
    tm.tm_year = 0;
    EXPECT_EQ("00", generate("%y"));
    EXPECT_EQ(common::using_strftime("%y", tm), generate("%y"));
}

TEST_F(year_generator_test_case_t, FirstTwoDigits) {
    tm.tm_year = 114;
    EXPECT_EQ("20", generate("%C"));
    EXPECT_EQ(common::using_strftime("%C", tm), generate("%C"));
}

TEST_F(year_generator_test_case_t, FirstTwoDigitsLowerBound) {
    tm.tm_year = 0;
    EXPECT_EQ("19", generate("%C"));
    EXPECT_EQ(common::using_strftime("%C", tm), generate("%C"));
}

TEST_F(year_generator_test_case_t, FirstTwoDigitsUpperBound) {
    tm.tm_year = 200;
    EXPECT_EQ("21", generate("%C"));
    EXPECT_EQ(common::using_strftime("%C", tm), generate("%C"));
}

TEST_F(year_generator_test_case_t, FullYearWithSuffixLiteral) {
    tm.tm_year = 114;
    EXPECT_EQ("2014-", generate("%Y-"));
    EXPECT_EQ(common::using_strftime("%Y-", tm), generate("%Y-"));
}

TEST_F(year_generator_test_case_t, FullYearWithPrefixLiteral) {
    tm.tm_year = 114;
    EXPECT_EQ("-2014", generate("-%Y"));
    EXPECT_EQ(common::using_strftime("-%Y", tm), generate("-%Y"));
}

TEST_F(year_generator_test_case_t, FullYearWithPrefixAndSuffixLiteral) {
    tm.tm_year = 114;
    EXPECT_EQ("-2014-", generate("-%Y-"));
    EXPECT_EQ(common::using_strftime("-%Y-", tm), generate("-%Y-"));
}
