#include <gtest/gtest.h>

#include <rapidjson/document.h>

#include <blackhole/config/json.hpp>

namespace blackhole {
namespace testing {

using config::json_t;

TEST(json_t, IsNil) {
    const auto json = "null";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_TRUE(config.is_nil());
}

TEST(json_t, IsBool) {
    const auto json = "true";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_nil());
    EXPECT_TRUE(config.is_bool());
    EXPECT_FALSE(config.is_i64());
    EXPECT_FALSE(config.is_u64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, ToBool) {
    const auto json = "true";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_TRUE(config.to_bool());
}

// TODO: is_string, is_array, is_object.
// TODO: to_string.
// TODO: to<T>(default) for all cases.

TEST(json_t, ThrowsExceptionOnBoolMismatch) {
    const auto json = "42";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_bool(), bad_cast);
}

TEST(json_t, IsInt) {
    const auto json = "42";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_nil());
    EXPECT_FALSE(config.is_bool());
    EXPECT_TRUE(config.is_i64());
    EXPECT_TRUE(config.is_u64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, ToInt) {
    const auto json = "42";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_EQ(42, config.to_i64());
    EXPECT_EQ(42, config.to_u64());
}

TEST(json_t, ThrowsExceptionOnIntMismatch) {
    const auto json = "false";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_i64(), bad_cast);
    EXPECT_THROW(config.to_u64(), bad_cast);
}

TEST(json_t, IsDouble) {
    const auto json = "42.5";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_nil());
    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_i64());
    EXPECT_FALSE(config.is_u64());
    EXPECT_TRUE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, ToDouble) {
    const auto json = "42.5";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_DOUBLE_EQ(42.5, config.to_double());
}

TEST(json_t, ThrowsExceptionOnDoubleMismatch) {
    const auto json = "false";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_double(), bad_cast);
}

TEST(json_t, IsString) {
    const auto json = R"("le value")";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_nil());
    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_i64());
    EXPECT_FALSE(config.is_u64());
    EXPECT_FALSE(config.is_double());
    EXPECT_TRUE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, ToString) {
    const auto json = R"("le value")";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_EQ("le value", config.to_string());
}

TEST(json_t, ThrowsExceptionOnStringMismatch) {
    const auto json = "false";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_string(), bad_cast);
}

TEST(config_t, json) {
    const auto json = R"({
        "formatter": {
            "type": "string",
            "pattern": "[%(level)s]: %(message)s"
        },
        "sinks": [
            {
                "type": "files",
                "path": "test.log",
                "rotation": {
                    "pattern": "test.log.%N",
                    "backups": 5,
                    "size": 1000000
                }
            }
        ]
    })";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_EQ("string", config["formatter"]["type"]->to_string());
    EXPECT_EQ("[%(level)s]: %(message)s", config["formatter"]["pattern"]->to_string());

    EXPECT_EQ("files", config["sinks"][0]["type"]->to_string());
    EXPECT_EQ("test.log", config["sinks"][0]["path"]->to_string());
    EXPECT_EQ("test.log.%N", config["sinks"][0]["rotation"]["pattern"]->to_string());
    EXPECT_EQ(5, config["sinks"][0]["rotation"]["backups"]->to_i64());
    EXPECT_EQ(1000000, config["sinks"][0]["rotation"]["size"]->to_u64());

    // NOTE: There is only one sink, so it's okay to compare strongly.
    auto counter = 0;
    config["sinks"]->each([&](const config_t& sink) {
        EXPECT_EQ("files", sink["type"]->to_string());
        EXPECT_EQ("test.log", sink["path"]->to_string());
        EXPECT_EQ("test.log.%N", sink["rotation"]["pattern"]->to_string());
        EXPECT_EQ(5, sink["rotation"]["backups"]->to_i64());
        EXPECT_EQ(1000000, sink["rotation"]["size"]->to_u64());
        ++counter;
    });

    EXPECT_EQ(1, counter);

    std::vector<std::string> keys;
    config.each_map([&](const std::string& key, const config_t& sink) {
        keys.push_back(key);
    });

    EXPECT_EQ((std::vector<std::string>{"formatter", "sinks"}), keys);
}

}  // namespace testing
}  // namespace blackhole

// TODO: It's likely that I'll drop all is_* methods for unnecessity.
