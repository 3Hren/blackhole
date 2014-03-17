#include "common.hpp"

class day_of_month_generator_test_case_t : public generator_test_case_t {};

TEST_F(day_of_month_generator_test_case_t, Numeric) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%d"));
    EXPECT_EQ(common::using_strftime("%d", tm), generate("%d"));
}

TEST_F(day_of_month_generator_test_case_t, NumericSingleDigit) {
    tm.tm_mday = 6;
    EXPECT_EQ("06", generate("%d"));
    EXPECT_EQ(common::using_strftime("%d", tm), generate("%d"));
}

TEST_F(day_of_month_generator_test_case_t, NumericNewPlaceholder) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%e"));
    EXPECT_EQ(common::using_strftime("%e", tm), generate("%e"));
}

TEST_F(day_of_month_generator_test_case_t, NumericNewPlaceholderSingleDigitIsPrecededBySpace) {
    tm.tm_mday = 1;
    EXPECT_EQ(" 1", generate("%e"));
    EXPECT_EQ(common::using_strftime("%e", tm), generate("%e"));
}
