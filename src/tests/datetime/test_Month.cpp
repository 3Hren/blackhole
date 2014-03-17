#include "common.hpp"

class month_generator_test_case_t : public generator_test_case_t {};

TEST_F(month_generator_test_case_t, Numeric) {
    tm.tm_mon = 2;
    EXPECT_EQ("03", generate("%m"));
    EXPECT_EQ(common::using_strftime("%m", tm), generate("%m"));
}

TEST_F(month_generator_test_case_t, Abbreviated) {
    tm.tm_mon = 2;
    EXPECT_EQ("Mar", generate("%b"));
    EXPECT_EQ(common::using_strftime("%b", tm), generate("%b"));
}

TEST_F(month_generator_test_case_t, AbbreviatedSynonym) {
    // Assuming English locale is set.
    tm.tm_mon = 2;
    EXPECT_EQ("Mar", generate("%h"));
    EXPECT_EQ(common::using_strftime("%h", tm), generate("%h"));
}

TEST_F(month_generator_test_case_t, Full) {
    // Assuming English locale is set.
    tm.tm_mon = 2;
    EXPECT_EQ("March", generate("%B"));
    EXPECT_EQ(common::using_strftime("%B", tm), generate("%B"));
}
