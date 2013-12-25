#include "Mocks.hpp"

TEST(json_t, Class) {
    formatter::json_t fmt;
    UNUSED(fmt);
}

TEST(json_t, FormatSingleAttribute) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message"
    };

    formatter::json_t fmt;
    std::string expected = "{\"message\":\"le message\"}";
    EXPECT_EQ(expected, fmt.format(record));
}

TEST(json_t, FormatMultipleAttributes) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = 100500
    };

    formatter::json_t fmt;
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    EXPECT_STREQ("le message", doc["message"].GetString());
    EXPECT_EQ(100500, doc["timestamp"].GetInt());
}
