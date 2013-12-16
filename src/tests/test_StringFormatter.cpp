#include "Mocks.hpp"

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
    EXPECT_TRUE(actual.find("'answer to life the universe and everything': '42'") != std::string::npos);
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
    EXPECT_TRUE(actual.find("'answer to life the universe and everything': '42'") != std::string::npos);
    EXPECT_TRUE(actual.find("'uuid': '123-456'") != std::string::npos);
}

namespace testing {

std::string map_timestamp(const log::attribute_value_t& value) {
    std::time_t time = boost::get<std::time_t>(value);
    char mbstr[128];
    if(std::strftime(mbstr, 128, "%F %T", std::localtime(&time))) {
        return std::string(mbstr);
    }
    return std::string("?");
}

} // namespace testing

TEST(string_t, CustomMapping) {
    log::record_t record;
    record.attributes = {
        keyword::timestamp() = std::time_t(100500),
        keyword::message() = "le message",
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t formatter(pattern);
    formatter.add_mapper("timestamp", &testing::map_timestamp);
    std::string actual = formatter.format(record);
    EXPECT_EQ(actual, "[1970-01-02 06:55:00]: le message");
}
