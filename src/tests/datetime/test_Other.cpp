#include "common.hpp"

TEST_F(generator_test_case_t, ComplexFormatting) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ("2014-02-23 12:20:30", generate("%Y-%m-%d %H:%M:%S"));
    EXPECT_EQ(common::using_strftime("%Y-%m-%d %H:%M:%S", tm), generate("%Y-%m-%d %H:%M:%S"));
}

TEST_F(generator_test_case_t, StandardDateTime) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ(common::using_strftime("%c", tm), generate("%c"));
}

TEST_F(generator_test_case_t, DateISO8601) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    EXPECT_EQ("2014-02-23", generate("%F"));
    EXPECT_EQ(common::using_strftime("%F", tm), generate("%F"));
}

TEST_F(generator_test_case_t, OffsetFromUTC) {
    tm.tm_isdst = 3;
    EXPECT_EQ(common::using_strftime("%z", tm), generate("%z"));
}

TEST_F(generator_test_case_t, TimeZone) {
    tm.tm_isdst = 3;
    EXPECT_EQ(common::using_strftime("%Z", tm), generate("%Z"));
}

TEST_F(generator_test_case_t, PercentSign) {
    tm.tm_isdst = 3;
    EXPECT_EQ(common::using_strftime("%%", tm), generate("%%"));
}

TEST_F(generator_test_case_t, MonthDayYear) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    EXPECT_EQ(common::using_strftime("%m/%d/%y", tm), generate("%D"));
    EXPECT_EQ(common::using_strftime("%D", tm), generate("%D"));
}

TEST_F(generator_test_case_t, TimeISO8601) {
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ(common::using_strftime("%H:%M:%S", tm), generate("%T"));
    EXPECT_EQ(common::using_strftime("%T", tm), generate("%T"));
}
