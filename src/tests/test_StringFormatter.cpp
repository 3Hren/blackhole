#include <blackhole/formatter/string.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(string_t, FormatSingleAttribute) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };
    std::string pattern("[%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le message]", fmt.format(record));
}

TEST(string_t, FormatMultipleAttributes) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        { "timestamp", log::attribute_t("le timestamp") }
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST(string_t, FormatMultipleAttributesMoreThanExists) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        { "timestamp", log::attribute_t("le timestamp") },
        { "source", log::attribute_t("le source") }
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST(string_t, ThrowsExceptionWhenAttributeNameNotProvided) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_THROW(fmt.format(record), blackhole::error_t);
}

TEST(string_t, FormatOtherLocalAttribute) {
    log::record_t record;
    record.attributes = {
        { "uuid", log::attribute_t("123-456") },
    };
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("['uuid': '123-456']", formatter.format(record));
}

TEST(string_t, FormatOtherLocalAttributes) {
    log::record_t record;
    record.attributes = {
        { "uuid", log::attribute_t("123-456") },
        { "answer to life the universe and everything", log::attribute_t(42) }
    };
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    std::string actual = formatter.format(record);
    EXPECT_TRUE(actual.find("'answer to life the universe and everything': 42") != std::string::npos);
    EXPECT_TRUE(actual.find("'uuid': '123-456'") != std::string::npos);
}

TEST(string_t, ComplexFormatWithOtherLocalAttributes) {
    log::record_t record;
    record.attributes = {
        { "timestamp", log::attribute_t("1960-01-01 00:00:00", log::attribute::scope::event) },
        { "level", log::attribute_t("INFO", log::attribute::scope::event) },
        keyword::message() = "le message",
        { "uuid", log::attribute_t("123-456") },
        { "answer to life the universe and everything", log::attribute_t(42) }
    };
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

    log::record_t record;
    record.attributes = {
        keyword::timestamp() = timeval { 100500, 0 },
        keyword::message() = "le message",
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t formatter(pattern);
    formatter.set_mapper(std::move(mapper));
    std::string actual = formatter.format(record);
    EXPECT_EQ(actual, "[1970-01-02 03:55:00.000000]: le message");
}

TEST(string_t, CustomMappingWithKeyword) {
    mapping::value_t mapper;
    mapper.add<keyword::tag::timestamp_t>(&testing::map_timestamp);

    log::record_t record;
    record.attributes = {
        keyword::timestamp() = timeval { 100500, 0 },
        keyword::message() = "le message",
    };
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

//!@todo: Extended string formatter spec:
/*!
    Tests:
        1. One attribute.
        2. Two attributes.
        3. Zero attributes.
        4. Throws if array.size <= variadics.

    Sample config:
[
    {
        "pattern": "%(key)s : %(value)s",
        "guard": {"key": "'", "value": "smart", "escape": ['"', '=']}
        "open": "[",
        "close": "]",
        "separator": ","
    }
]
*/
