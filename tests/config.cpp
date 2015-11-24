#include <blackhole/config.hpp>

#include <boost/assert.hpp>

namespace blackhole {

/// None value. Throws an exception on any get access, but maps to none on subscription.
class none_t : public config_t {
public:
    auto operator[](const std::size_t& idx) const -> monadic<config_t>;
    auto operator[](const std::string& key) const -> monadic<config_t>;

    auto is_nil() const -> bool;
    auto to_bool() const -> bool;
    auto to_i64() const -> std::int64_t;
    auto to_u64() const -> std::uint64_t;
    auto to_string() const -> std::string;

    auto each(const each_function& fn) -> void;
    auto each_map(const member_function& fn) -> void;
};

// TODO: Can be hidden for the sake of encapsulation.
template<>
class monadic<config_t> {
    std::unique_ptr<config_t> inner;

public:
    monadic() :
        inner(new none_t)
    {}

    explicit monadic(std::unique_ptr<config_t> inner) :
        inner(std::move(inner))
    {
        BOOST_ASSERT(this->inner);
    }

    auto operator[](const std::size_t& idx) const -> monadic<config_t> {
        return inner->operator[](idx);
    }

    auto operator[](const std::string& key) const -> monadic<config_t> {
        return inner->operator[](key);
    }

    auto operator->() -> config_t* {
        return inner.get();
    }

    auto operator*() const -> const config_t& {
        return *inner;
    }
};

auto
none_t::operator[](const std::size_t& idx) const -> monadic<config_t> {
    return {};
}

auto
none_t::operator[](const std::string& key) const -> monadic<config_t> {
    return {};
}

auto
none_t::is_nil() const -> bool {
    return true;
}

auto
none_t::to_bool() const -> bool {
    throw bad_optional_access();
}

auto
none_t::to_i64() const -> std::int64_t {
    throw std::out_of_range("none");
}

auto
none_t::to_u64() const -> std::uint64_t {
    throw std::out_of_range("none");
}

auto
none_t::to_string() const -> std::string {
    throw std::out_of_range("none");
}

auto
none_t::each(const each_function& fn) -> void {
    throw std::out_of_range("none");
}

auto
none_t::each_map(const member_function& fn) -> void {
    throw std::out_of_range("none");
}

template<typename T, typename... Args>
auto make_monadic(Args&&... args) -> monadic<config_t> {
    return monadic<config_t>(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}  // namespace blackhole

#include <gtest/gtest.h>

#include <rapidjson/document.h>

namespace blackhole {
namespace testing {

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

    auto operator[](const std::size_t& idx) const -> monadic<config_t> {
        if (value.IsArray() && idx < value.Size()) {
            return make_monadic<json_t>(value[idx]);
        }

        return {};
    }

    auto operator[](const std::string& key) const -> monadic<config_t> {
        if (value.IsObject() && value.HasMember(key.c_str())) {
            return make_monadic<json_t>(value[key.c_str()]);
        }

        return {};
    }

    auto is_nil() const -> bool {
        return value.IsNull();
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

        throw std::invalid_argument(""); // TODO: Bad cast.
    }

    auto to_u64() const -> std::uint64_t {
        if (value.IsUint64()) {
            return value.GetUint64();
        }

        throw std::invalid_argument(""); // TODO: Bad cast.
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

TEST(null_t, IsNil) {
    none_t config;

    EXPECT_TRUE(config.is_nil());
}

TEST(null_t, ThrowsOnEveryGetterInvocation) {
    none_t config;

    EXPECT_THROW(config.to_bool(), bad_optional_access);
}

TEST(json_t, IsNil) {
    const auto json = "null";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_TRUE(config.is_nil());
}

TEST(json_t, ToBool) {
    const auto json = "true";

    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_TRUE(config.to_bool());
}

// TODO: is_bool, is_i64, is_u64, is_double, is_string, is_array, is_object.
// TODO: to_i64, to_u64, to_double, to_string.
// TODO: for null, for json (check also bad casts).
// TODO: to<T>(default) for all cases.

TEST(json_t, ThrowsExceptionOnBoolMismatch) {
    const auto json = "42";
    rapidjson::Document doc;
    doc.Parse<0>(json);
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_THROW(config.to_bool(), bad_cast);
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
