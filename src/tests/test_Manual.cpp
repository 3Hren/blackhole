#include <blackhole/detail/datetime.hpp>

#include "global.hpp"

using namespace blackhole::aux::datetime;

class generator_test_case_t : public Test {
protected:
    std::tm tm;

    void SetUp() {
        tm = std::tm();
    }

    std::string generate(const std::string& pattern) {
        std::string str;
        blackhole::aux::attachable_basic_ostringstream<char> stream(str);
        generator_t generator = generator_factory_t::make(pattern);
        generator(stream, tm);
        return stream.str();
    }
};

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

class month_generator_test_case_t : public generator_test_case_t {};

TEST_F(month_generator_test_case_t, Numeric) {
    tm.tm_mon = 2;
    EXPECT_EQ("03", generate("%m"));
}

TEST_F(month_generator_test_case_t, Abbreviated) {
    tm.tm_mon = 2;
    EXPECT_EQ("Mar", generate("%b"));
}

TEST_F(month_generator_test_case_t, AbbreviatedSynonym) {
    // Assuming English locale is set.
    tm.tm_mon = 2;
    EXPECT_EQ("Mar", generate("%h"));
}

TEST_F(month_generator_test_case_t, Full) {
    // Assuming English locale is set.
    tm.tm_mon = 2;
    EXPECT_EQ("March", generate("%B"));
}

TEST_F(generator_test_case_t, NumericDayOfMonth) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%d"));
}

TEST_F(generator_test_case_t, NumericDayOfMonthWithSingleDigit) {
    tm.tm_mday = 6;
    EXPECT_EQ("06", generate("%d"));
}

TEST_F(generator_test_case_t, ShortNumericDayOfMonth) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%e"));
}

TEST_F(generator_test_case_t, FullDayHour) {
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%H"));
}

TEST_F(generator_test_case_t, FullDayHourWithSingleDigit) {
    tm.tm_hour = 0;
    EXPECT_EQ("00", generate("%H"));
}

TEST_F(generator_test_case_t, HalfDayHour) {
    //!@note: tm_hour is treated as [0; 23], so 00:30 will be 12:30.
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourWithSingleDigit) {
    tm.tm_hour = 6;
    EXPECT_EQ("06", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourWithOverflow) {
    tm.tm_hour = 13;
    EXPECT_EQ("01", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourLowerBorderCase) {
    tm.tm_hour = 0;
    EXPECT_EQ("12", generate("%I"));
}

TEST_F(generator_test_case_t, HalfDayHourUpperBorderCase) {
    tm.tm_hour = 12;
    EXPECT_EQ("12", generate("%I"));
}

TEST_F(generator_test_case_t, Minute) {
    tm.tm_min = 30;
    EXPECT_EQ("30", generate("%M"));
}

TEST_F(generator_test_case_t, MinuteLowerBound) {
    tm.tm_min = 00;
    EXPECT_EQ("00", generate("%M"));
}

TEST_F(generator_test_case_t, MinuteUpperBound) {
    tm.tm_min = 59;
    EXPECT_EQ("59", generate("%M"));
}

TEST_F(generator_test_case_t, Second) {
    tm.tm_sec = 30;
    EXPECT_EQ("30", generate("%S"));
}

TEST_F(generator_test_case_t, SecondLowerBound) {
    tm.tm_sec = 0;
    EXPECT_EQ("00", generate("%S"));
}

TEST_F(generator_test_case_t, SecondUpperBound) {
    tm.tm_sec = 60;
    EXPECT_EQ("60", generate("%S"));
}

TEST_F(generator_test_case_t, ComplexFormatting) {
    tm.tm_year = 2014;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ("2014-02-23 12:20:30", generate("%Y-%m-%d %H:%M:%S"));
}

using namespace blackhole::aux;

TEST(ostringstreambuf, Class) {
    ostringstreambuf buf;
    UNUSED(buf);
}

TEST(ostringstreambuf, StoreDataString) {
    std::string storage;
    ostringstreambuf streambuf(storage);
    std::ostream stream(&streambuf);
    stream << "Blah";
    EXPECT_EQ("Blah", storage);
}

TEST(ostringstreambuf, StoreDataStringLongerThanInitialSize) {
    std::string storage;
    ostringstreambuf streambuf(storage);
    std::ostream stream(&streambuf);
    stream << "Blahblahblahblah-blah!";
    EXPECT_EQ("Blahblahblahblah-blah!", storage);
}

TEST(ostringstreambuf, CanAttachString) {
    std::string storage;
    ostringstreambuf streambuf;
    std::ostream stream(&streambuf);
    streambuf.attach(storage);
    stream << "Blah";
    EXPECT_EQ("Blah", storage);
}
