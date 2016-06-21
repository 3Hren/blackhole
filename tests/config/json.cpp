#include <gtest/gtest.h>

#include <boost/optional/optional.hpp>

#include <rapidjson/document.h>

#include <blackhole/config/json.hpp>

#include <src/config/json.hpp>

namespace blackhole {
namespace testing {

using config::json_t;

TEST(json_t, IsBool) {
    const auto json = "true";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_TRUE(config.is_bool());
    EXPECT_FALSE(config.is_sint64());
    EXPECT_FALSE(config.is_uint64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, IsInt) {
    const auto json = "42";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_bool());
    EXPECT_TRUE(config.is_sint64());
    EXPECT_TRUE(config.is_uint64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, IsDouble) {
    const auto json = "42.5";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_sint64());
    EXPECT_FALSE(config.is_uint64());
    EXPECT_TRUE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, IsString) {
    const auto json = R"("le value")";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_sint64());
    EXPECT_FALSE(config.is_uint64());
    EXPECT_FALSE(config.is_double());
    EXPECT_TRUE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, IsVector) {
    const auto json = "[1,2,3]";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_sint64());
    EXPECT_FALSE(config.is_uint64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_TRUE(config.is_vector());
    EXPECT_FALSE(config.is_object());
}

TEST(json_t, IsObject) {
    const auto json = "{}";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_FALSE(config.is_bool());
    EXPECT_FALSE(config.is_sint64());
    EXPECT_FALSE(config.is_uint64());
    EXPECT_FALSE(config.is_double());
    EXPECT_FALSE(config.is_string());
    EXPECT_FALSE(config.is_vector());
    EXPECT_TRUE(config.is_object());
}

TEST(json_t, ToBool) {
    const auto json = "true";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_TRUE(config.to_bool());
}

TEST(json_t, ToInt) {
    const auto json = "42";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_EQ(42, config.to_sint64());
    EXPECT_EQ(42, config.to_uint64());
}

TEST(json_t, ToDouble) {
    const auto json = "42.5";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_DOUBLE_EQ(42.5, config.to_double());
}

TEST(json_t, ToString) {
    const auto json = R"("le value")";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_EQ("le value", config.to_string());
}

TEST(json_t, ThrowsExceptionOnBoolMismatch) {
    const auto json = "42";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_bool(), std::logic_error);
}

TEST(json_t, ThrowsExceptionOnIntMismatch) {
    const auto json = "false";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_sint64(), std::logic_error);
    EXPECT_THROW(config.to_uint64(), std::logic_error);
}

TEST(json_t, ThrowsExceptionOnDoubleMismatch) {
    const auto json = "false";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_double(), std::logic_error);
}

TEST(json_t, ThrowsExceptionOnStringMismatch) {
    const auto json = "false";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_string(), std::logic_error);
}

TEST(json_t, complex) {
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

    EXPECT_EQ("string", *config["formatter"]["type"].to_string());
    EXPECT_EQ("[%(level)s]: %(message)s", *config["formatter"]["pattern"].to_string());

    EXPECT_EQ("files", *config["sinks"][0]["type"].to_string());
    EXPECT_EQ("test.log", *config["sinks"][0]["path"].to_string());
    EXPECT_EQ("test.log.%N", *config["sinks"][0]["rotation"]["pattern"].to_string());
    EXPECT_EQ(5, *config["sinks"][0]["rotation"]["backups"].to_sint64());
    EXPECT_EQ(1000000, *config["sinks"][0]["rotation"]["size"].to_uint64());

    // NOTE: There is only one sink, so it's okay to compare strongly.
    auto counter = 0;
    config["sinks"].each([&](const config::node_t& sink) {
        EXPECT_EQ("files", *sink["type"].to_string());
        EXPECT_EQ("test.log", *sink["path"].to_string());
        EXPECT_EQ("test.log.%N", *sink["rotation"]["pattern"].to_string());
        EXPECT_EQ(5, *sink["rotation"]["backups"].to_sint64());
        EXPECT_EQ(1000000, *sink["rotation"]["size"].to_uint64());
        ++counter;
    });

    EXPECT_EQ(1, counter);

    std::vector<std::string> keys;
    config.each_map([&](const std::string& key, const config::node_t& sink) {
        keys.push_back(key);
    });

    EXPECT_EQ((std::vector<std::string>{"formatter", "sinks"}), keys);
}

TEST(json_t, ToBoolActualInt) {
    const auto json = "3.1415";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    try {
        config.to_bool();
        FAIL();
    } catch (const config::type_mismatch& err) {
        EXPECT_EQ("/", err.cursor());
        EXPECT_EQ("bool", err.expected());
        EXPECT_EQ("number", err.actual());
        EXPECT_STREQ(R"(type mismatch at "/": expected "bool", actual "number")", err.what());
    }
}

TEST(json_t, ToNumberActualStringWithMultipleIndex) {
    const auto json = R"({
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

    try {
        config["sinks"][0]["rotation"]["pattern"].to_sint64().get();
        FAIL();
    } catch (const config::type_mismatch& err) {
        EXPECT_EQ("/sinks/0/rotation/pattern", err.cursor());
        EXPECT_EQ("int64", err.expected());
        EXPECT_EQ("string", err.actual());
        EXPECT_STREQ(R"(type mismatch at "/sinks/0/rotation/pattern": expected "int64", actual "string")", err.what());
    }
}

TEST(factory, FromInvalidJson) {
    std::stringstream stream;
    stream << R"({"key": "value)";

    try {
        config::factory<json_t> factory(stream);
        FAIL();
    } catch (const std::invalid_argument& err) {
        EXPECT_STREQ("parse error at offset 13: Missing a closing quotation mark in string.", err.what());
    }
}

}  // namespace testing
}  // namespace blackhole
