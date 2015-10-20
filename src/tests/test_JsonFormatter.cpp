#include <blackhole/formatter/json.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"
#include "util/unused.hpp"

using namespace blackhole;

TEST(json_t, Class) {
    formatter::json_t fmt;
    UNUSED(fmt);
}

TEST(json_t, FormatSingleAttribute) {
    record_t record;
    record.insert(keyword::message() = "le message");

    formatter::json_t fmt;
    std::string expected = "{\"message\":\"le message\"}";
    EXPECT_EQ(expected, fmt.format(record));
}

TEST(json_t, FormatMultipleAttributes) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

    formatter::json_t fmt;
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("le message", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    ASSERT_TRUE(doc["timestamp"].IsInt());
    EXPECT_EQ(100500, doc["timestamp"].GetInt());
}

TEST(json_t, FormatMultipleComplexAttributes) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });
    record.insert(attribute::make("@source", "udp://127.0.0.1"));
    record.insert(attribute::make("@source_host", "dhcp-218-248-wifi.yandex"));
    record.insert(attribute::make("@source_path", "service/storage"));
    record.insert(attribute::make("@uuid", "550e8400-e29b-41d4-a716-446655440000"));

    formatter::json_t fmt;
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    ASSERT_TRUE(doc.HasMember("message"));
    ASSERT_TRUE(doc["message"].IsString());
    EXPECT_STREQ("le message", doc["message"].GetString());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    ASSERT_TRUE(doc["timestamp"].IsInt());
    EXPECT_EQ(100500, doc["timestamp"].GetInt());

    ASSERT_TRUE(doc.HasMember("@source"));
    ASSERT_TRUE(doc["@source"].IsString());
    EXPECT_STREQ("udp://127.0.0.1", doc["@source"].GetString());

    ASSERT_TRUE(doc.HasMember("@source_host"));
    ASSERT_TRUE(doc["@source_host"].IsString());
    EXPECT_STREQ("dhcp-218-248-wifi.yandex", doc["@source_host"].GetString());

    ASSERT_TRUE(doc.HasMember("@source_path"));
    ASSERT_TRUE(doc["@source_path"].IsString());
    EXPECT_STREQ("service/storage", doc["@source_path"].GetString());

    ASSERT_TRUE(doc.HasMember("@uuid"));
    ASSERT_TRUE(doc["@uuid"].IsString());
    EXPECT_STREQ("550e8400-e29b-41d4-a716-446655440000", doc["@uuid"].GetString());
}

TEST(json_t, SingleAttributeMapping) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

    formatter::json_t::config_type config;
    config.naming["message"] = "@message";

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    ASSERT_TRUE(doc.HasMember("@message"));
    ASSERT_TRUE(doc["@message"].IsString());
    EXPECT_STREQ("le message", doc["@message"].GetString());

    ASSERT_TRUE(doc.HasMember("timestamp"));
    ASSERT_TRUE(doc["timestamp"].IsInt());
    EXPECT_EQ(100500, doc["timestamp"].GetInt());
}

TEST(json_t, MultipleAttributeMapping) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

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
    record_t record;
    record.insert(keyword::message() = "le message");

    formatter::json_t::config_type config;
    config.newline = true;

    formatter::json_t fmt(config);
    std::string actual = fmt.format(record);

    rapidjson::Document doc;
    doc.Parse<0>(actual.c_str());
    EXPECT_TRUE(boost::ends_with(actual, "\n"));
}

TEST(json_t, FieldMapping) {
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

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
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

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
    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(keyword::timestamp() = timeval{ 100500, 0 });

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

void map_secret_value(blackhole::stickystream_t& stream, std::uint32_t value) {
    stream << "(" << value << ")";
}

} // namespace testing

TEST(json_t, AttributeMappingIsDeterminedByItsBaseNames) {
    mapping::value_t mapper;
    mapper.add<std::uint32_t>("secret", &testing::map_secret_value);

    record_t record;
    record.insert(keyword::message() = "le message");
    record.insert(attribute::make<std::uint32_t>("secret", 42));

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

TEST(json_t, AllowDuplicateAttributes) {
    record_t record;
    record.insert(keyword::message() = "le message1");
    record.insert(keyword::message() = "le message2");

    formatter::json_t fmt;
    std::string expected = "{\"message\":\"le message1\",\"message\":\"le message2\"}";
    EXPECT_EQ(expected, fmt.format(record));
}

TEST(json_t, DisallowDuplicateAttributesIfNeeded) {
    record_t record;
    record.insert(keyword::message() = "le message1");
    record.insert(keyword::message() = "le message2");

    formatter::json::config_t config;
    config.filter = true;

    formatter::json_t fmt(config);
    std::string expected = "{\"message\":\"le message2\"}";
    EXPECT_EQ(expected, fmt.format(record));
}
