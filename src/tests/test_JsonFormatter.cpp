#include <blackhole/formatter/json.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

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
        keyword::timestamp() = timeval{ 100500, 0 }
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
        keyword::timestamp() = timeval{ 100500, 0 },
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
        keyword::timestamp() = timeval{ 100500, 0 }
    };

    formatter::json_t::config_type config;
    config.naming["message"] = "@message";

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    ASSERT_TRUE(doc.HasMember("@message"));
    EXPECT_STREQ("le message", doc["@message"].GetString());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    EXPECT_EQ(100500, doc["timestamp"].GetInt());
}

TEST(json_t, MultipleAttributeMapping) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = timeval{ 100500, 0 }
    };

    formatter::json_t::config_type config;
    config.naming["message"] = "@message";
    config.naming["timestamp"] = "@timestamp";

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    ASSERT_TRUE(doc.HasMember("@message"));
    EXPECT_STREQ("le message", doc["@message"].GetString());

    ASSERT_TRUE(doc.HasMember("@timestamp"));
    EXPECT_EQ(100500, doc["@timestamp"].GetInt());
}

TEST(json_t, AddsNewlineIfSpecified) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
    };

    formatter::json_t::config_type config;
    config.newline = true;

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    EXPECT_TRUE(boost::ends_with(actual, "\n"));
}

TEST(json_t, FieldMapping) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = timeval{ 100500, 0 }
    };

    formatter::json_t::config_type config;
    config.routing.specified["timestamp"] = { "fields" };

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());

    ASSERT_TRUE(doc.HasMember("message"));

    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("timestamp"));
    EXPECT_EQ(100500, doc["fields"]["timestamp"].GetInt());
}

TEST(json_t, ComplexFieldMapping) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = timeval{ 100500, 0 }
    };

    formatter::json_t::config_type config;
    config.routing.specified["message"] = { "fields" };
    config.routing.specified["timestamp"] = { "fields", "aux" };

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());

    ASSERT_TRUE(doc.HasMember("fields"));
    ASSERT_TRUE(doc["fields"].HasMember("message"));
    EXPECT_STREQ("le message", doc["fields"]["message"].GetString());

    ASSERT_TRUE(doc["fields"].HasMember("aux"));
    ASSERT_TRUE(doc["fields"]["aux"].HasMember("timestamp"));
    EXPECT_EQ(100500, doc["fields"]["aux"]["timestamp"].GetInt());
}

TEST(json_t, UnspecifiedPositionalMapping) {
    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        keyword::timestamp() = timeval{ 100500, 0 }
    };

    formatter::json_t::config_type config;
    config.routing.unspecified = { "fields" };

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());

    ASSERT_TRUE(doc.HasMember("fields"));

    ASSERT_TRUE(doc["fields"].HasMember("message"));
    EXPECT_STREQ("le message", doc["fields"]["message"].GetString());

    ASSERT_TRUE(doc["fields"].HasMember("timestamp"));
    EXPECT_EQ(100500, doc["fields"]["timestamp"].GetInt());
}

namespace testing {

void map_secret_value(blackhole::aux::attachable_ostringstream& stream, std::uint32_t value) {
    stream << "(" << value << ")";
}

} // namespace testing

TEST(json_t, AttributeMappingIsDeterminedByItsBaseNames) {
    mapping::value_t mapper;
    mapper.add<std::uint32_t>("secret", &testing::map_secret_value);

    log::record_t record;
    record.attributes = {
        keyword::message() = "le message",
        attribute::make<std::uint32_t>("secret", 42)
    };

    formatter::json_t::config_type config;
    config.naming["message"] = "@message";
    config.naming["secret"] = "@secret";

    formatter::json_t fmt(config);
    fmt.set_mapper(std::move(mapper));
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());

    ASSERT_TRUE(doc.HasMember("@message"));
    ASSERT_TRUE(doc["@message"].IsString());
    EXPECT_STREQ("le message", doc["@message"].GetString());

    ASSERT_TRUE(doc.HasMember("@secret"));
    ASSERT_TRUE(doc["@secret"].IsString());
    EXPECT_STREQ("(42)", doc["@secret"].GetString());
}
