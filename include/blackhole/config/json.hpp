#pragma once

#include "blackhole/config.hpp"
#include "blackhole/config/monadic.hpp"

namespace blackhole {
namespace config {

// TODO: It's a detail!
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
            fn(*make_monadic<json_t>(*it));
        }
    }

    auto each_map(const member_function& fn) -> void {
        if (!value.IsObject()) {
            throw bad_cast();
        }

        for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
            fn(it->name.GetString(), *make_monadic<json_t>(it->value));
        }
    }
};

}  // namespace config
}  // namespace blackhole
