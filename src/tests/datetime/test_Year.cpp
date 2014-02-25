#include "common.hpp"

class year_generator_test_case_t : public generator_test_case_t {};

TEST_F(year_generator_test_case_t, FullYear) {
    tm.tm_year = 2014;
    EXPECT_EQ("2014", generate("%Y"));
}

TEST_F(year_generator_test_case_t, ShortYear) {
    tm.tm_year = 2014;
    EXPECT_EQ("14", generate("%y"));
}

TEST_F(year_generator_test_case_t, ShortYearWithZeroPrefix) {
    tm.tm_year = 2004;
    EXPECT_EQ("04", generate("%y"));
}

TEST_F(year_generator_test_case_t, ShortYearMinValue) {
    tm.tm_year = 2000;
    EXPECT_EQ("00", generate("%y"));
}

TEST_F(year_generator_test_case_t, ShortYearFirstTwoDigits) {
    tm.tm_year = 2014;
    EXPECT_EQ("20", generate("%C"));
}

TEST_F(year_generator_test_case_t, ShortYearFirstTwoDigitsLowerBound) {
    tm.tm_year = 0;
    EXPECT_EQ("00", generate("%C"));
}

TEST_F(year_generator_test_case_t, ShortYearFirstTwoDigitsUpperBound) {
    tm.tm_year = 9914;
    EXPECT_EQ("99", generate("%C"));
}

TEST_F(year_generator_test_case_t, FullYearWithSuffixLiteral) {
    tm.tm_year = 2014;
    EXPECT_EQ("2014-", generate("%Y-"));
}

TEST_F(year_generator_test_case_t, FullYearWithPrefixLiteral) {
    tm.tm_year = 2014;
    EXPECT_EQ("-2014", generate("-%Y"));
}

TEST_F(year_generator_test_case_t, FullYearWithPrefixAndSuffixLiteral) {
    tm.tm_year = 2014;
    EXPECT_EQ("-2014-", generate("-%Y-"));
}
