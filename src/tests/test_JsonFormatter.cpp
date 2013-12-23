#include "Mocks.hpp"

TEST(json_t, Class) {
    formatter::json_t fmt;
    UNUSED(fmt);
}

TEST_OFF(json_t, FormatSingleAttribute) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };

    formatter::json_t fmt;
    std::string expected = "{\"message\":\"le message\"}\n";
    EXPECT_EQ(expected, fmt.format(record));
}

TEST_OFF(json_t, FormatMultipleAttributes) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        { "timestamp", log::attribute_t("le timestamp") }
    };

    formatter::json_t fmt;
    std::string expected = "{\"message\":\"le message\",\"timestamp\":\"le timestamp\"}\n";
    EXPECT_EQ(expected, fmt.format(record));
}
