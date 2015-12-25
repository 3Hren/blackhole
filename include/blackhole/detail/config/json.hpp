#pragma once

#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include <rapidjson/document.h>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"

namespace blackhole {
namespace detail {
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

    ~type_mismatch() throw() {}

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

// TODO: Separate hpp/cpp.
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

    auto to_bool() const -> bool {
        if (value.IsBool()) {
            return value.GetBool();
        }

        type_mismatch("bool");
    }

    auto to_sint64() const -> std::int64_t {
        if (value.IsInt64()) {
            return value.GetInt64();
        }

        type_mismatch("int64");
    }

    auto to_uint64() const -> std::uint64_t {
        if (value.IsUint64()) {
            return value.GetUint64();
        }

        type_mismatch("uint64");
    }

    auto to_double() const -> double {
        if (value.IsDouble()) {
            return value.GetDouble();
        }

        type_mismatch("double");
    }

    auto to_string() const -> std::string {
        if (value.IsString()) {
            return value.GetString();
        }

        type_mismatch("string");
    }

    auto each(const each_function& fn) -> void {
        if (!value.IsArray()) {
            type_mismatch("array");
        }

        for (auto it = value.Begin(); it != value.End(); ++it) {
            fn(json_t(*it, advance(static_cast<std::size_t>(std::distance(value.Begin(), it)))));
        }
    }

    auto each_map(const member_function& fn) -> void {
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

    __attribute((noreturn)) auto type_mismatch(const std::string& expected) const -> void {
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

}  // namespace config
}  // namespace detail
}  // namespace blackhole
