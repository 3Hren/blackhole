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

TEST(json_t, FormatMultipleComplexAttributes) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = 100500,
        attribute::make("@source", "udp://127.0.0.1"),
        attribute::make("@source_host", "dhcp-218-248-wifi.yandex"),
        attribute::make("@source_path", "service/storage"),
        attribute::make("@uuid", "550e8400-e29b-41d4-a716-446655440000")
    };

    formatter::json_t fmt;
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    EXPECT_STREQ("le message", doc["message"].GetString());
    EXPECT_EQ(100500, doc["timestamp"].GetInt());
    EXPECT_STREQ("udp://127.0.0.1", doc["@source"].GetString());
    EXPECT_STREQ("dhcp-218-248-wifi.yandex", doc["@source_host"].GetString());
    EXPECT_STREQ("service/storage", doc["@source_path"].GetString());
    EXPECT_STREQ("550e8400-e29b-41d4-a716-446655440000", doc["@uuid"].GetString());
}

TEST(json_t, SingleAttributeMapping) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = 100500
    };

    formatter::json_t::config_type config;
    config.mapping["message"] = "@message";

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    ASSERT_TRUE(doc.HasMember("@message"));
    EXPECT_STREQ("le message", doc["@message"].GetString());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    EXPECT_EQ(100500, doc["timestamp"].GetInt());
}
