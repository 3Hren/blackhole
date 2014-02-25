#include "common.hpp"

//! \brief Depends on `tm_yday` - days since January 1 – [0, 365].
//!        Value tm.tm_yday=0 means 1 day for `%j` placeholder.

class day_of_year_generator_test_case_t : public generator_test_case_t {};

TEST_F(day_of_year_generator_test_case_t, Numeric) {
    tm.tm_yday = 300;
    EXPECT_EQ("301", generate("%j"));
    EXPECT_EQ(common::using_strftime("%j", tm), generate("%j"));
}
