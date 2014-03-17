#include "common.hpp"

//! \brief Depends on `tm_year, tm_wday and tm_yday`.

class weak_generator_test_case_t : public generator_test_case_t {};

TEST_F(weak_generator_test_case_t, SundayFirstDayOfTheWeek) {
    tm.tm_year = 114;
    tm.tm_wday = 3;
    tm.tm_yday = 100;
    EXPECT_EQ(common::using_strftime("%U", tm), generate("%U"));
}

TEST_F(weak_generator_test_case_t, MondayFirstDayOfTheWeek) {
    tm.tm_year = 114;
    tm.tm_wday = 3;
    tm.tm_yday = 100;
    EXPECT_EQ(common::using_strftime("%W", tm), generate("%W"));
}
