#pragma once

#include <rapidjson/document.h>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"

namespace blackhole {
namespace detail {
namespace config {

template<typename T>
using option = blackhole::config::option<T>;

using blackhole::config::make_option;
using blackhole::config::node_t;

class bad_cast : public std::logic_error {
public:
    // TODO: Add line:column.
    bad_cast() : std::logic_error("bad cast") {}
};

// TODO: Separate hpp/cpp.
class json_t : public node_t {
    const rapidjson::Value& value;

public:
    explicit json_t(const rapidjson::Value& value) :
        value(value)
    {}

    auto to_bool() const -> bool {
        if (value.IsBool()) {
            return value.GetBool();
        }

        throw bad_cast();
    }

    auto to_sint64() const -> std::int64_t {
        if (value.IsInt64()) {
            return value.GetInt64();
        }

        throw bad_cast();
    }

    auto to_uint64() const -> std::uint64_t {
        if (value.IsUint64()) {
            return value.GetUint64();
        }

        throw bad_cast();
    }

    auto to_double() const -> double {
        if (value.IsDouble()) {
            return value.GetDouble();
        }

        throw bad_cast();
    }

    auto to_string() const -> std::string {
        if (value.IsString()) {
            return value.GetString();
        }

        throw bad_cast();
    }

    auto each(const each_function& fn) -> void {
        if (!value.IsArray()) {
            throw bad_cast();
        }

        for (auto it = value.Begin(); it != value.End(); ++it) {
            fn(json_t(*it));
        }
    }

    auto each_map(const member_function& fn) -> void {
        if (!value.IsObject()) {
            throw bad_cast();
        }

        for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
            fn(it->name.GetString(), json_t(it->value));
        }
    }

    auto operator[](const std::size_t& idx) const -> option<node_t> {
        if (value.IsArray() && idx < value.Size()) {
            return make_option<json_t>(value[static_cast<rapidjson::SizeType>(idx)]);
        }

        return {};
    }

    auto operator[](const std::string& key) const -> option<node_t> {
        if (value.IsObject() && value.HasMember(key.c_str())) {
            return make_option<json_t>(value[key.c_str()]);
        }

        return {};
    }
};

}  // namespace config
}  // namespace detail
}  // namespace blackhole
