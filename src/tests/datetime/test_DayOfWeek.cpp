#include "common.hpp"

//! \brief Depends on `tm_wday`: days since Sunday – [0, 6].

class day_of_week_generator_test_case_t : public generator_test_case_t {};

TEST_F(day_of_week_generator_test_case_t, Abbreviate) {
    for (int i = 0; i < 7; ++i) {
        tm.tm_mday = 0;
        EXPECT_EQ(common::using_strftime("%a", tm), generate("%a"));
    }
}

TEST_F(day_of_week_generator_test_case_t, FullWeeklyName) {
    for (int i = 0; i < 7; ++i) {
        tm.tm_mday = 0;
        EXPECT_EQ(common::using_strftime("%A", tm), generate("%A"));
    }
}
