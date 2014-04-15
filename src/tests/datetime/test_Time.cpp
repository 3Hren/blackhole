#include "common.hpp"

class time_generator_test_case_t : public generator_test_case_t {};

TEST_F(time_generator_test_case_t, FullDayHour) {
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%H"));
    EXPECT_EQ(common::using_strftime("%H", tm), generate("%H"));
}

TEST_F(time_generator_test_case_t, FullDayHourWithSingleDigit) {
    tm.tm_hour = 0;
    EXPECT_EQ("00", generate("%H"));
    EXPECT_EQ(common::using_strftime("%H", tm), generate("%H"));
}

TEST_F(time_generator_test_case_t, HalfDayHour) {
    //!@note: tm_hour is treated as [0; 23], so 00:30 will be 12:30.
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%I"));
    EXPECT_EQ(common::using_strftime("%I", tm), generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourWithSingleDigit) {
    tm.tm_hour = 6;
    EXPECT_EQ("06", generate("%I"));
    EXPECT_EQ(common::using_strftime("%I", tm), generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourWithOverflow) {
    tm.tm_hour = 13;
    EXPECT_EQ("01", generate("%I"));
    EXPECT_EQ(common::using_strftime("%I", tm), generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourLowerBorderCase) {
    tm.tm_hour = 0;
    EXPECT_EQ("12", generate("%I"));
    EXPECT_EQ(common::using_strftime("%I", tm), generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourUpperBorderCase) {
    tm.tm_hour = 12;
    EXPECT_EQ("12", generate("%I"));
    EXPECT_EQ(common::using_strftime("%I", tm), generate("%I"));
}

TEST_F(time_generator_test_case_t, Minute) {
    tm.tm_min = 30;
    EXPECT_EQ("30", generate("%M"));
    EXPECT_EQ(common::using_strftime("%M", tm), generate("%M"));
}

TEST_F(time_generator_test_case_t, MinuteLowerBound) {
    tm.tm_min = 00;
    EXPECT_EQ("00", generate("%M"));
    EXPECT_EQ(common::using_strftime("%M", tm), generate("%M"));
}

TEST_F(time_generator_test_case_t, MinuteUpperBound) {
    tm.tm_min = 59;
    EXPECT_EQ("59", generate("%M"));
    EXPECT_EQ(common::using_strftime("%M", tm), generate("%M"));
}

TEST_F(time_generator_test_case_t, Second) {
    tm.tm_sec = 30;
    EXPECT_EQ("30", generate("%S"));
    EXPECT_EQ(common::using_strftime("%S", tm), generate("%S"));
}

TEST_F(time_generator_test_case_t, SecondLowerBound) {
    tm.tm_sec = 0;
    EXPECT_EQ("00", generate("%S"));
    EXPECT_EQ(common::using_strftime("%S", tm), generate("%S"));
}

TEST_F(time_generator_test_case_t, SecondUpperBound) {
    tm.tm_sec = 60;
    EXPECT_EQ("60", generate("%S"));
    EXPECT_EQ(common::using_strftime("%S", tm), generate("%S"));
}

TEST_F(time_generator_test_case_t, Am) {
    tm.tm_hour = 11;
    EXPECT_EQ(common::using_strftime("%p", tm), generate("%p"));
}

TEST_F(time_generator_test_case_t, Pm) {
    tm.tm_hour = 13;
    EXPECT_EQ(common::using_strftime("%p", tm), generate("%p"));
}

TEST_F(time_generator_test_case_t, Microsecond) {
    usec = 100500;
    EXPECT_EQ("100500", generate("%f"));
}

TEST_F(time_generator_test_case_t, MicrosecondLowerBould) {
    usec = 0;
    EXPECT_EQ("000000", generate("%f"));
}

TEST_F(time_generator_test_case_t, MicrosecondUpperBould) {
    usec = 999999;
    EXPECT_EQ("999999", generate("%f"));
}

TEST_F(time_generator_test_case_t, MicrosecondIncomplete) {
    usec = 42;
    EXPECT_EQ("000042", generate("%f"));
}

