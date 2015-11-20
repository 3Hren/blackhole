#include <gtest/gtest.h>

#include <string>

#include <boost/assert.hpp>

#include <cppformat/format.h>

#include <blackhole/detail/datetime.hpp>

namespace blackhole {
namespace testing {

using blackhole::detail::datetime::make_generator;

class datetime_t : public ::testing::Test {
protected:
    std::tm tm;
    std::uint64_t usec;

    void SetUp() {
        tm = std::tm();
        usec = 0;
    }

    auto generate(const std::string& pattern) const -> std::string {
        fmt::MemoryWriter wr;

        const auto generator = make_generator(pattern);
        generator(wr, tm, usec);
        return {wr.data(), wr.size()};
    }
};

namespace {

template<std::size_t N>
inline std::string using_strftime(const char(&format)[N], const std::tm& tm) {
    char buffer[128];
    std::size_t ret = std::strftime(buffer, sizeof(buffer), format, &tm);
    BOOST_ASSERT(ret > 0);

    return std::string(buffer, ret);
}

}  // namespace

TEST_F(datetime_t, DayOfMonthNum) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%d"));
    EXPECT_EQ(using_strftime("%d", tm), generate("%d"));
}

TEST_F(datetime_t, DayOfMonthNumSingleDigit) {
    tm.tm_mday = 6;
    EXPECT_EQ("06", generate("%d"));
    EXPECT_EQ(using_strftime("%d", tm), generate("%d"));
}

TEST_F(datetime_t, DayOfMonthNumNewPlaceholder) {
    tm.tm_mday = 23;
    EXPECT_EQ("23", generate("%e"));
    EXPECT_EQ(using_strftime("%e", tm), generate("%e"));
}

TEST_F(datetime_t, DayOfMonthNumNewPlaceholderSingleDigitIsPrecededBySpace) {
    tm.tm_mday = 1;
    EXPECT_EQ(" 1", generate("%e"));
    EXPECT_EQ(using_strftime("%e", tm), generate("%e"));
}

TEST_F(datetime_t, DayOfWeekAbbreviate) {
    for (int i = 0; i < 7; ++i) {
        tm.tm_mday = 0;
        EXPECT_EQ(using_strftime("%a", tm), generate("%a"));
    }
}

TEST_F(datetime_t, DayOfWeekFullWeeklyName) {
    for (int i = 0; i < 7; ++i) {
        tm.tm_mday = 0;
        EXPECT_EQ(using_strftime("%A", tm), generate("%A"));
    }
}

TEST_F(datetime_t, DayOfYearNumeric) {
    tm.tm_yday = 300;
    EXPECT_EQ("301", generate("%j"));
    EXPECT_EQ(using_strftime("%j", tm), generate("%j"));
}

TEST_F(datetime_t, MonthNumeric) {
    tm.tm_mon = 2;
    EXPECT_EQ("03", generate("%m"));
    EXPECT_EQ(using_strftime("%m", tm), generate("%m"));
}

TEST_F(datetime_t, MonthAbbreviated) {
    tm.tm_mon = 2;
    EXPECT_EQ(using_strftime("%b", tm), generate("%b"));
}

TEST_F(datetime_t, MonthAbbreviatedSynonym) {
    tm.tm_mon = 2;
    EXPECT_EQ(using_strftime("%h", tm), generate("%h"));
}

TEST_F(datetime_t, MonthFull) {
    tm.tm_mon = 2;
    EXPECT_EQ(using_strftime("%B", tm), generate("%B"));
}

TEST_F(datetime_t, ComplexFormatting) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ("2014-02-23 12:20:30", generate("%Y-%m-%d %H:%M:%S"));
    EXPECT_EQ(using_strftime("%Y-%m-%d %H:%M:%S", tm), generate("%Y-%m-%d %H:%M:%S"));
}

TEST_F(datetime_t, StandardDateTime) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ(using_strftime("%c", tm), generate("%c"));
}

TEST_F(datetime_t, DateISO8601) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    EXPECT_EQ("2014-02-23", generate("%F"));
    EXPECT_EQ(using_strftime("%F", tm), generate("%F"));
}

TEST_F(datetime_t, OffsetFromUTC) {
    tm.tm_isdst = 3;
    EXPECT_EQ(using_strftime("%z", tm), generate("%z"));
}

TEST_F(datetime_t, TimeZone) {
    tm.tm_isdst = 3;
    EXPECT_EQ(using_strftime("%Z", tm), generate("%Z"));
}

TEST_F(datetime_t, PercentSign) {
    tm.tm_isdst = 3;
    EXPECT_EQ(using_strftime("%%", tm), generate("%%"));
}

TEST_F(datetime_t, MonthDayYear) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    EXPECT_EQ(using_strftime("%m/%d/%y", tm), generate("%D"));
    EXPECT_EQ(using_strftime("%D", tm), generate("%D"));
}

TEST_F(datetime_t, TimeISO8601) {
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;
    EXPECT_EQ(using_strftime("%H:%M:%S", tm), generate("%T"));
    EXPECT_EQ(using_strftime("%T", tm), generate("%T"));
}

TEST_F(datetime_t, FullDayHour) {
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%H"));
    EXPECT_EQ(using_strftime("%H", tm), generate("%H"));
}

TEST_F(datetime_t, FullDayHourWithSingleDigit) {
    tm.tm_hour = 0;
    EXPECT_EQ("00", generate("%H"));
    EXPECT_EQ(using_strftime("%H", tm), generate("%H"));
}

TEST_F(datetime_t, HalfDayHour) {
    //!@note: tm_hour is treated as [0; 23], so 00:30 will be 12:30.
    tm.tm_hour = 11;
    EXPECT_EQ("11", generate("%I"));
    EXPECT_EQ(using_strftime("%I", tm), generate("%I"));
}

TEST_F(datetime_t, HalfDayHourWithSingleDigit) {
    tm.tm_hour = 6;
    EXPECT_EQ("06", generate("%I"));
    EXPECT_EQ(using_strftime("%I", tm), generate("%I"));
}

TEST_F(datetime_t, HalfDayHourWithOverflow) {
    tm.tm_hour = 13;
    EXPECT_EQ("01", generate("%I"));
    EXPECT_EQ(using_strftime("%I", tm), generate("%I"));
}

TEST_F(datetime_t, HalfDayHourLowerBorderCase) {
    tm.tm_hour = 0;
    EXPECT_EQ("12", generate("%I"));
    EXPECT_EQ(using_strftime("%I", tm), generate("%I"));
}

TEST_F(datetime_t, HalfDayHourUpperBorderCase) {
    tm.tm_hour = 12;
    EXPECT_EQ("12", generate("%I"));
    EXPECT_EQ(using_strftime("%I", tm), generate("%I"));
}

TEST_F(datetime_t, Minute) {
    tm.tm_min = 30;
    EXPECT_EQ("30", generate("%M"));
    EXPECT_EQ(using_strftime("%M", tm), generate("%M"));
}

TEST_F(datetime_t, MinuteLowerBound) {
    tm.tm_min = 00;
    EXPECT_EQ("00", generate("%M"));
    EXPECT_EQ(using_strftime("%M", tm), generate("%M"));
}

TEST_F(datetime_t, MinuteUpperBound) {
    tm.tm_min = 59;
    EXPECT_EQ("59", generate("%M"));
    EXPECT_EQ(using_strftime("%M", tm), generate("%M"));
}

TEST_F(datetime_t, Second) {
    tm.tm_sec = 30;
    EXPECT_EQ("30", generate("%S"));
    EXPECT_EQ(using_strftime("%S", tm), generate("%S"));
}

TEST_F(datetime_t, SecondLowerBound) {
    tm.tm_sec = 0;
    EXPECT_EQ("00", generate("%S"));
    EXPECT_EQ(using_strftime("%S", tm), generate("%S"));
}

TEST_F(datetime_t, SecondUpperBound) {
    tm.tm_sec = 60;
    EXPECT_EQ("60", generate("%S"));
    EXPECT_EQ(using_strftime("%S", tm), generate("%S"));
}

TEST_F(datetime_t, Am) {
    tm.tm_hour = 11;
    EXPECT_EQ(using_strftime("%p", tm), generate("%p"));
}

TEST_F(datetime_t, Pm) {
    tm.tm_hour = 13;
    EXPECT_EQ(using_strftime("%p", tm), generate("%p"));
}

TEST_F(datetime_t, Microsecond) {
    usec = 100500;
    EXPECT_EQ("100500", generate("%f"));
}

TEST_F(datetime_t, MicrosecondLowerBould) {
    usec = 0;
    EXPECT_EQ("000000", generate("%f"));
}

TEST_F(datetime_t, MicrosecondUpperBould) {
    usec = 999999;
    EXPECT_EQ("999999", generate("%f"));
}

TEST_F(datetime_t, MicrosecondIncomplete) {
    usec = 42;
    EXPECT_EQ("000042", generate("%f"));
}

TEST_F(datetime_t, SundayFirstDayOfTheWeek) {
    tm.tm_year = 114;
    tm.tm_wday = 3;
    tm.tm_yday = 100;
    EXPECT_EQ(using_strftime("%U", tm), generate("%U"));
}

TEST_F(datetime_t, MondayFirstDayOfTheWeek) {
    tm.tm_year = 114;
    tm.tm_wday = 3;
    tm.tm_yday = 100;
    EXPECT_EQ(using_strftime("%W", tm), generate("%W"));
}

TEST_F(datetime_t, FullYear) {
    tm.tm_year = 114;
    EXPECT_EQ("2014", generate("%Y"));
    EXPECT_EQ(using_strftime("%Y", tm), generate("%Y"));
}

TEST_F(datetime_t, ShortYear) {
    tm.tm_year = 114;
    EXPECT_EQ("14", generate("%y"));
    EXPECT_EQ(using_strftime("%y", tm), generate("%y"));
}

TEST_F(datetime_t, ShortYearWithZeroPrefix) {
    tm.tm_year = 104;
    EXPECT_EQ("04", generate("%y"));
    EXPECT_EQ(using_strftime("%y", tm), generate("%y"));
}

TEST_F(datetime_t, ShortYearMinValue) {
    tm.tm_year = 0;
    EXPECT_EQ("00", generate("%y"));
    EXPECT_EQ(using_strftime("%y", tm), generate("%y"));
}

TEST_F(datetime_t, FirstTwoDigits) {
    tm.tm_year = 114;
    EXPECT_EQ("20", generate("%C"));
    EXPECT_EQ(using_strftime("%C", tm), generate("%C"));
}

TEST_F(datetime_t, FirstTwoDigitsLowerBound) {
    tm.tm_year = 0;
    EXPECT_EQ("19", generate("%C"));
    EXPECT_EQ(using_strftime("%C", tm), generate("%C"));
}

TEST_F(datetime_t, FirstTwoDigitsUpperBound) {
    tm.tm_year = 200;
    EXPECT_EQ("21", generate("%C"));
    EXPECT_EQ(using_strftime("%C", tm), generate("%C"));
}

TEST_F(datetime_t, FullYearWithSuffixLiteral) {
    tm.tm_year = 114;
    EXPECT_EQ("2014-", generate("%Y-"));
    EXPECT_EQ(using_strftime("%Y-", tm), generate("%Y-"));
}

TEST_F(datetime_t, FullYearWithPrefixLiteral) {
    tm.tm_year = 114;
    EXPECT_EQ("-2014", generate("-%Y"));
    EXPECT_EQ(using_strftime("-%Y", tm), generate("-%Y"));
}

TEST_F(datetime_t, FullYearWithPrefixAndSuffixLiteral) {
    tm.tm_year = 114;
    EXPECT_EQ("-2014-", generate("-%Y-"));
    EXPECT_EQ(using_strftime("-%Y-", tm), generate("-%Y-"));
}

TEST_F(datetime_t, Large) {
    tm.tm_year = 114;
    tm.tm_mon = 1;
    tm.tm_mday = 23;
    tm.tm_hour = 12;
    tm.tm_min = 20;
    tm.tm_sec = 30;

    std::string pattern("%Y-%m-%d %H:%M:%S");
    std::string expected("2014-02-23 12:20:30");
    for (int i = 0; i < 40; ++i) {
        pattern  += " || %Y-%m-%d %H:%M:%S";
        expected += " || 2014-02-23 12:20:30";
    }

    EXPECT_EQ(expected, generate(pattern));
}

}  // namespace testing
}  // namespace blackhole
