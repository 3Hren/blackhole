#include "common.hpp"

class time_generator_test_case_t : public generator_test_case_t {};

TEST_F(time_generator_test_case_t, FullDayHour) {
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%H"));
}

TEST_F(time_generator_test_case_t, FullDayHourWithSingleDigit) {
    tm.tm_hour = 0;
    EXPECT_EQ("00", generate("%H"));
}

TEST_F(time_generator_test_case_t, HalfDayHour) {
    //!@note: tm_hour is treated as [0; 23], so 00:30 will be 12:30.
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourWithSingleDigit) {
    tm.tm_hour = 6;
    EXPECT_EQ("06", generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourWithOverflow) {
    tm.tm_hour = 13;
    EXPECT_EQ("01", generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourLowerBorderCase) {
    tm.tm_hour = 0;
    EXPECT_EQ("12", generate("%I"));
}

TEST_F(time_generator_test_case_t, HalfDayHourUpperBorderCase) {
    tm.tm_hour = 12;
    EXPECT_EQ("12", generate("%I"));
}

TEST_F(time_generator_test_case_t, Minute) {
    tm.tm_min = 30;
    EXPECT_EQ("30", generate("%M"));
}

TEST_F(time_generator_test_case_t, MinuteLowerBound) {
    tm.tm_min = 00;
    EXPECT_EQ("00", generate("%M"));
}

TEST_F(time_generator_test_case_t, MinuteUpperBound) {
    tm.tm_min = 59;
    EXPECT_EQ("59", generate("%M"));
}

TEST_F(time_generator_test_case_t, Second) {
    tm.tm_sec = 30;
    EXPECT_EQ("30", generate("%S"));
}

TEST_F(time_generator_test_case_t, SecondLowerBound) {
    tm.tm_sec = 0;
    EXPECT_EQ("00", generate("%S"));
}

TEST_F(time_generator_test_case_t, SecondUpperBound) {
    tm.tm_sec = 60;
    EXPECT_EQ("60", generate("%S"));
}
