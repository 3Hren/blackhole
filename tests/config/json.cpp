#include <blackhole/config.hpp>
#include <blackhole/config/monadic.hpp>
#include <blackhole/config/none.hpp>

#include <boost/assert.hpp>

#include <gtest/gtest.h>

#include <rapidjson/document.h>

namespace blackhole {
namespace testing {

using config::none_t;
using config::make_monadic;

// builder(xml) - читает и генерирует свое дерево и реализации config_t.
//  .sevmap(...) - просто запоминает
//  .build() - ищет ключ "formatter/type" config["formatter"]["type"]
//             по type делегирует config["formatter"] в нужную фабрику

class json_t : public config_t {
    const rapidjson::Value& value;

public:
    explicit json_t(const rapidjson::Value& value) :
        value(value)
    {}

    auto operator[](const std::size_t& idx) const -> config::monadic<config_t> {
        if (value.IsArray() && idx < value.Size()) {
            return make_monadic<json_t>(value[idx]);
        }

        return {};
    }

    auto operator[](const std::string& key) const -> config::monadic<config_t> {
        if (value.IsObject() && value.HasMember(key.c_str())) {
            return make_monadic<json_t>(value[key.c_str()]);
        }

        return {};
    }

    auto is_nil() const -> bool {
        return value.IsNull();
    }

    auto is_bool() const -> bool {
        return value.IsBool();
    }

    auto is_i64() const -> bool {
        return value.IsInt64();
    }

    auto is_u64() const -> bool {
        return value.IsUint64();
    }

    auto is_double() const -> bool {
        return value.IsDouble();
    }

    auto is_string() const -> bool {
        return value.IsString();
    }

    auto is_vector() const -> bool {
        return value.IsArray();
    }

    auto is_object() const -> bool {
        return value.IsObject();
    }

    auto to_bool() const -> bool {
        if (value.IsBool()) {
            return value.GetBool();
        }

        throw bad_cast();
    }

    auto to_i64() const -> std::int64_t {
        if (value.IsInt64()) {
            return value.GetInt64();
        }

        throw bad_cast();
    }

    auto to_u64() const -> std::uint64_t {
        if (value.IsUint64()) {
            return value.GetUint64();
        }

        throw bad_cast();
    }

    auto to_double() const -> double {
        if (is_double()) {
            return value.GetDouble();
        }

        throw bad_cast();
    }

    auto to_string() const -> std::string {
        if (value.IsString()) {
            return value.GetString();
        }

        throw std::invalid_argument(""); // TODO: Bad cast.
    }

    auto each(const each_function& fn) -> void {
        for (auto it = value.Begin(); it != value.End(); ++it) {
            fn(*make_monadic<json_t>(*it));
        }
    }

    auto each_map(const member_function& fn) -> void {
        for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
            fn(it->name.GetString(), *make_monadic<json_t>(it->value));
        }
    }
};

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

// TODO: is_double, is_string, is_array, is_object.
// TODO: to_double, to_string.
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
