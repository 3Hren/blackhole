#include "common.hpp"

TEST_F(generator_test_case_t, ComplexFormatting) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ("2014-02-23 12:20:30", generate("%Y-%m-%d %H:%M:%S"));
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
