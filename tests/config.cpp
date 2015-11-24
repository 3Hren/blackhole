#include <cstdint>
#include <memory>

#include <boost/assert.hpp>

namespace blackhole {

template<typename T>
class monadic;

class config_t {
public:
    virtual auto operator[](const std::size_t& idx) const -> monadic<config_t> = 0;
    virtual auto operator[](const std::string& key) const -> monadic<config_t> = 0;

    virtual auto to_i64() const -> std::int64_t = 0;
    virtual auto to_u64() const -> std::uint64_t = 0;
    virtual auto to_string() const -> std::string = 0;
};

// TODO: Can be hidden for the sake of encapsulation.
template<>
class monadic<config_t> {
    std::unique_ptr<config_t> inner;

public:
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
};

template<typename T, typename... Args>
auto make_monadic(Args&&... args) -> monadic<config_t> {
    return monadic<config_t>(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}  // namespace blackhole

#include <gtest/gtest.h>

#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>

namespace blackhole {
namespace testing {

// builder(xml) - читает и генерирует свое дерево и реализации config_t.
//  .sevmap(...) - просто запоминает
//  .build() - ищет ключ "formatter/type" config["formatter"]["type"]
//             по type делегирует config["formatter"] в нужную фабрику

/// None value. Throws an exception on any get access, but maps to none on subscription.
class none_t : public config_t {
    auto operator[](const std::size_t& idx) const -> monadic<config_t> {
        return make_monadic<none_t>();
    }

    auto operator[](const std::string& key) const -> monadic<config_t> {
        return make_monadic<none_t>();
    }

    auto to_i64() const -> std::int64_t {
        throw std::out_of_range("none");
    }

    auto to_u64() const -> std::uint64_t {
        throw std::out_of_range("none");
    }

    auto to_string() const -> std::string {
        throw std::out_of_range("none");
    }
};

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

        return make_monadic<none_t>();
    }

    auto operator[](const std::string& key) const -> monadic<config_t> {
        if (value.IsObject() && value.HasMember(key.c_str())) {
            return make_monadic<json_t>(value[key.c_str()]);
        }

        return make_monadic<none_t>();
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
};

class json_factory_t {
    rapidjson::Document doc;

public:
    explicit json_factory_t(const std::string& value) {
        doc.Parse<0>(value.c_str());
        if (doc.HasParseError()) {
            throw std::runtime_error("parse error");
        }
    }

    auto build() -> void {
        auto config = json_t(doc);
        const auto type = config["formatter"]["type"]->to_string();
    }
};

TEST(config_t, json) {
    static const std::string JSON = R"({
        'formatter': {
            'type': 'string',
            'pattern': '[%(level)s]: %(message)s'
        },
        'sinks': [
            {
                'type': 'files',
                'path': 'test.log',
                'rotation': {
                    'pattern': 'test.log.%N',
                    'backups': 5,
                    'size': 1000000
                }
            }
        ]
    })";

    const std::string valid(boost::algorithm::replace_all_copy(JSON, "'", "\""));
    rapidjson::Document doc;
    doc.Parse<0>(valid.c_str());
    ASSERT_FALSE(doc.HasParseError());

    json_t config(doc);

    EXPECT_EQ("string", config["formatter"]["type"]->to_string());
    EXPECT_EQ("[%(level)s]: %(message)s", config["formatter"]["pattern"]->to_string());

    EXPECT_EQ("files", config["sinks"][0]["type"]->to_string());
    EXPECT_EQ("test.log", config["sinks"][0]["path"]->to_string());
    EXPECT_EQ("test.log.%N", config["sinks"][0]["rotation"]["pattern"]->to_string());
    EXPECT_EQ(5, config["sinks"][0]["rotation"]["backups"]->to_i64());
    EXPECT_EQ(1000000, config["sinks"][0]["rotation"]["size"]->to_u64());
}

}  // namespace testing
}  // namespace blackhole
