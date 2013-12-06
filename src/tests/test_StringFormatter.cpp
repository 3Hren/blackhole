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

TEST(string_t, FormatOtherLocalAttributes) {
    log::record_t record;
    record.attributes = {
        { "uuid", { "123-456" } },
    };
    std::string pattern("[%(...L)s]");
    formatter::string_t formatter(pattern);
    EXPECT_EQ("['uuid': '123-456']", formatter.format(record));
}

//!@todo:
//! implement %(...A)s handling in formatter::string
//! implement %(...L)s handling for only local attributes in format::string
//! [L|S|G|T|U].
