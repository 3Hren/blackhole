#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <rapidjson/document.h>

#include "blackhole/compat.hpp"

#include "blackhole/config/factory.hpp"
#include "blackhole/config/json.hpp"
#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"

#include "factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace config {

using blackhole::config::make_option;
using blackhole::config::node_t;

typedef blackhole::config::option<node_t> option_t;

class type_mismatch : public std::logic_error {
    struct {
        std::string cursor;
        std::string expected;
        std::string actual;
    } data;

public:
    type_mismatch(std::string cursor, std::string expected, std::string actual) :
        std::logic_error("type mismatch at \"" + cursor + "\": " +
            "expected \"" + expected + "\", actual \"" + actual + "\"")
    {
        data.cursor = std::move(cursor);
        data.expected = std::move(expected);
        data.actual = std::move(actual);
    }

    type_mismatch(const type_mismatch& other) = default;
    type_mismatch(type_mismatch&& other) = default;

    ~type_mismatch() noexcept {}

    auto expected() const -> std::string {
        return data.expected;
    }

    auto actual() const -> std::string {
        return data.actual;
    }

    auto cursor() const -> std::string {
        return data.cursor;
    }
};

class json_t : public node_t {
    const rapidjson::Value& value;
    std::string cursor;

public:
    explicit json_t(const rapidjson::Value& value) :
        value(value)
    {}

    json_t(const rapidjson::Value& value, std::string cursor) :
        value(value),
        cursor(std::move(cursor))
    {}

    auto is_bool() const noexcept -> bool {
        return value.IsBool();
    }

    auto is_sint64() const noexcept -> bool {
        return value.IsInt64();
    }

    auto is_uint64() const noexcept -> bool {
        return value.IsUint64();
    }

    auto is_double() const noexcept -> bool {
        return value.IsDouble();
    }

    auto is_string() const noexcept -> bool {
        return value.IsString();
    }

    auto is_vector() const noexcept -> bool {
        return value.IsArray();
    }

    auto is_object() const noexcept -> bool {
        return value.IsObject();
    }

    auto to_bool() const -> bool {
        if (is_bool()) {
            return value.GetBool();
        }

        type_mismatch("bool");
    }

    auto to_sint64() const -> std::int64_t {
        if (is_sint64()) {
            return value.GetInt64();
        }

        type_mismatch("int64");
    }

    auto to_uint64() const -> std::uint64_t {
        if (is_uint64()) {
            return value.GetUint64();
        }

        type_mismatch("uint64");
    }

    auto to_double() const -> double {
        if (is_double()) {
            return value.GetDouble();
        }

        type_mismatch("double");
    }

    auto to_string() const -> std::string {
        if (is_string()) {
            return value.GetString();
        }

        type_mismatch("string");
    }

    auto each(const each_function& fn) const -> void {
        if (!value.IsArray()) {
            type_mismatch("array");
        }

        for (auto it = value.Begin(); it != value.End(); ++it) {
            fn(json_t(*it, advance(static_cast<std::size_t>(std::distance(value.Begin(), it)))));
        }
    }

    auto each_map(const member_function& fn) const -> void {
        if (!value.IsObject()) {
            type_mismatch("object");
        }

        for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
            fn(it->name.GetString(), json_t(it->value, advance(it->name.GetString())));
        }
    }

    auto operator[](const std::size_t& idx) const -> option_t {
        if (value.IsArray() && idx < value.Size()) {
            return make_option<json_t>(value[static_cast<rapidjson::SizeType>(idx)], advance(idx));
        }

        return {};
    }

    auto operator[](const std::string& key) const -> option_t {
        if (value.IsObject() && value.HasMember(key.c_str())) {
            return make_option<json_t>(value[key.c_str()], advance(key));
        }

        return {};
    }

private:
    auto advance(const std::size_t& idx) const -> std::string {
        return cursor + "/" + boost::lexical_cast<std::string>(idx);
    }

    auto advance(const std::string& key) const -> std::string {
        return cursor + "/" + key;
    }

    NORETURN auto type_mismatch(const std::string& expected) const -> void {
        throw config::type_mismatch(cursor.empty() ? "/" : cursor, expected, type());
    }

    auto type() const -> std::string {
        switch (value.GetType()) {
        case rapidjson::kNullType:
            return "null";
        case rapidjson::kTrueType:
        case rapidjson::kFalseType:
            return "bool";
        case rapidjson::kNumberType:
            return "number";
        case rapidjson::kStringType:
            return "string";
        case rapidjson::kArrayType:
            return "array";
        case rapidjson::kObjectType:
            return "object";
        }
    }
};

/// Represents a JSON based logger configuration factory.
template<>
class factory<json_t> : public factory_t {
    rapidjson::Document doc;
    const json_t node;

public:
    explicit factory(std::istream& stream);
    explicit factory(std::istream&& stream);

    /// Returns a const lvalue reference to the root configuration.
    auto config() const noexcept -> const node_t&;

private:
    auto initialize(std::istream& stream) -> void;
};

}  // namespace config
}  // namespace v1
}  // namespace blackhole
