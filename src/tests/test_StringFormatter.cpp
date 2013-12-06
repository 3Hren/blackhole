#include "Mocks.hpp"

using namespace blackhole;

TEST(string_t, FormatSingleAttribute) {
    log::record_t record;
    record.attributes = {
        { "message", { "le message" } }
    };
    std::string pattern("[%(message)s]");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le message]", fmt.format(record));
}

TEST(string_t, FormatMultipleAttributes) {
    log::record_t record;
    record.attributes = {
        { "message", { "le message" } },
        { "timestamp", { "le timestamp" } }
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST(string_t, FormatMultipleAttributesMoreThanExists) {
    log::record_t record;
    record.attributes = {
        { "message", { "le message" } },
        { "timestamp", { "le timestamp" } },
        { "source", { "le source" } }
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_EQ("[le timestamp]: le message", fmt.format(record));
}

TEST(string_t, ThrowsExceptionWhenAttributeNameNotProvided) {
    log::record_t record;
    record.attributes = {
        { "message", { "le message" } },
    };
    std::string pattern("[%(timestamp)s]: %(message)s");
    formatter::string_t fmt(pattern);
    EXPECT_THROW(fmt.format(record), error_t);
}

TEST(string_t, FormatOtherLocalAttribute) {
    log::record_t record;
    record.attributes = {
        { "uuid", { "123-456" } },
    };
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("['uuid': '123-456']", formatter.format(record));
}

TEST(string_t, FormatOtherLocalAttributes) {
    log::record_t record;
    record.attributes = {
        { "uuid", { "123-456" } },
        { "answer to life the universe and everything", { 42 } }
    };
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    // Result may vary depending on string hash function.
    EXPECT_EQ("['answer to life the universe and everything': '42', 'uuid': '123-456']", formatter.format(record));
}

TEST(string_t, ComplexFormatWithOtherLocalAttributes) {
    log::record_t record;
    record.attributes = {
        { "uuid", { "123-456" } },
        { "answer to life the universe and everything", { 42 } }
    };
    record.attributes["timestamp"] = { "1960-01-01 00:00:00", log::attribute_t::type_t::event };
    record.attributes["message"] = { "le message", log::attribute_t::type_t::event };
    record.attributes["level"] = { "INFO", log::attribute_t::type_t::event };
    std::string pattern("[%(timestamp)s] [%(level)s]: %(message)s [%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("[1960-01-01 00:00:00] [INFO]: le message ['answer to life the universe and everything': '42', 'uuid': '123-456']",
              formatter.format(record));
}

//!@todo:
//! implement %(...A)s handling in formatter::string
//! [L|E|G|T|U].
