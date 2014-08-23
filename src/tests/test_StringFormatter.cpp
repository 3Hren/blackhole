#include <blackhole/formatter/string.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(string_t, FormatSingleAttribute) {
    record_t record;
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le message]", fmt.format(record));
}

TEST(string_t, FormatMultipleAttributes) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert({ "timestamp", attribute_t("le timestamp") });

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST(string_t, FormatMultipleAttributesMoreThanExists) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert({ "timestamp", attribute_t("le timestamp") });
    record.insert({ "source", attribute_t("le source") });

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST(string_t, ThrowsExceptionWhenAttributeNameNotProvided) {
    record_t record;
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_THROW(fmt.format(record), blackhole::error_t);
}

TEST(string_t, FormatOtherLocalAttribute) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });

    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("['uuid': '123-456']", formatter.format(record));
}

TEST(string_t, FormatOtherLocalAttributes) {
    record_t record;
    record.insert({ "uuid", attribute_t("123-456") });
    record.insert({ "answer to life the universe and everything", attribute_t(42) });
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    std::string actual = formatter.format(record);
    EXPECT_TRUE(actual.find("'answer to life the universe and everything': 42") != std::string::npos);
    EXPECT_TRUE(actual.find("'uuid': '123-456'") != std::string::npos);
}

TEST(string_t, ComplexFormatWithOtherLocalAttributes) {
    record_t record;
    record.insert({ "timestamp", attribute_t("1960-01-01 00:00:00", attribute::scope_t::event) });
    record.insert({ "level", attribute_t("INFO", attribute::scope_t::event) });
    record.insert(keyword::message() = "le message");
    record.insert({ "uuid", attribute_t("123-456") });
    record.insert({ "answer to life the universe and everything", attribute_t(42) });

    std::string pattern("[%(timestamp)s] [%(level)s]: %(message)s [%(...L)s]");
    formatter::string_t formatter(pattern);
    std::string actual = formatter.format(record);
    EXPECT_TRUE(boost::starts_with(actual, "[1960-01-01 00:00:00] [INFO]: le message ["));
    EXPECT_TRUE(actual.find("'answer to life the universe and everything': 42") != std::string::npos);
    EXPECT_TRUE(actual.find("'uuid': '123-456'") != std::string::npos);
}

namespace testing {

void map_timestamp(blackhole::aux::attachable_ostringstream& stream, const timeval& tv) {
    char str[64];

    struct tm tm;
    gmtime_r((time_t *)&tv.tv_sec, &tm);
    if (std::strftime(str, sizeof(str), "%F %T", &tm)) {
        char usecs[64];
        snprintf(usecs, sizeof(usecs), ".%06ld", (long)tv.tv_usec);
        stream << str << usecs;
    } else {
        stream << "UNKNOWN";
    }
}

} // namespace testing

TEST(string_t, CustomMapping) {
    mapping::value_t mapper;
    mapper.add<timeval>("timestamp", &testing::map_timestamp);

    record_t record;
    record.insert(keyword::timestamp() = timeval { 100500, 0 });
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t formatter(pattern);
    formatter.set_mapper(std::move(mapper));
    std::string actual = formatter.format(record);
    EXPECT_EQ(actual, "[1970-01-02 03:55:00.000000]: le message");
}

TEST(string_t, CustomMappingWithKeyword) {
    mapping::value_t mapper;
    mapper.add<keyword::tag::timestamp_t>(&testing::map_timestamp);

    record_t record;
    record.insert(keyword::timestamp() = timeval { 100500, 0 });
    record.insert(keyword::message() = "le message");

    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t formatter(pattern);
    formatter.set_mapper(std::move(mapper));
    std::string actual = formatter.format(record);
    EXPECT_EQ(actual, "[1970-01-02 03:55:00.000000]: le message");
}

TEST(mapping, DatetimeMapping) {
    mapping::value_t mapper;
    mapper.add<keyword::tag::timestamp_t>("%Y-%m-%d %H:%M:%S");

    std::string result;
    aux::attachable_ostringstream stream;
    stream.attach(result);
    timeval tv{ 100500, 0 };
    mapper(stream, "timestamp", tv);
    EXPECT_EQ("1970-01-02 03:55:00", result);
}

#define TEST_STRING_FORMATTER(Suite, Case) \
    TEST(string_t##_##Suite, Case)

TEST_STRING_FORMATTER(Optional, Keyword) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%([id])?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<42>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentKeyword) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%([id])?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithPrefix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(.[id])?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<.42>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithPrefix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(.[id])?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%([id].)?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<42.>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%([id].)?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithPrefixSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(.[id].)?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<.42.>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithPrefixSuffix) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(.[id].)?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithPrefixSuffixParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(([id]))?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<(42)>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithPrefixSuffixParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(([id]))?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithPrefixSuffixReverseParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%()[id]()?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<)42(>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithPrefixSuffixReverseParentheses) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%()[id]()?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithPrefixSuffixSquareBrackets) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(\\[[id]\\])?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<[42]>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithPrefixSuffixSquareBrackets) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(\\[[id]\\])?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, WithPrefixSuffixSquareBracketsReversed) {
    record_t record;
    record.insert(attribute::make("message", "le message"));
    record.insert(attribute::make("id", 42));

    std::string pattern("<%(\\][id]\\[)?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<]42[>: [le message]", fmt.format(record));
}

TEST_STRING_FORMATTER(Optional, AbsentWithPrefixSuffixSquareBracketsReversed) {
    record_t record;
    record.insert(attribute::make("message", "le message"));

    std::string pattern("<%(\\][id]\\[)?s>: [%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("<>: [le message]", fmt.format(record));
}
