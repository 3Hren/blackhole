#pragma once

#include <rapidjson/document.h>

#include "blackhole/repository/config/parser.hpp"

namespace blackhole {

namespace repository {

namespace config {

// Converter adapter specializations for rapidjson value.
template<>
struct transformer_t<rapidjson::Value> {
    typedef rapidjson::Value value_type;

    static dynamic_t transform(const value_type& value) {
        switch (value.GetType()) {
        case rapidjson::kNullType:
            throw blackhole::error_t("null values are not supported");
        case rapidjson::kFalseType:
        case rapidjson::kTrueType:
            return value.GetBool();
        case rapidjson::kNumberType: {
            if (value.IsInt()) {
                return value.GetInt();
            } else if (value.IsInt64()) {
                return value.GetInt64();
            } else if (value.IsUint()) {
                return value.GetUint();
            } else if (value.IsUint64()) {
                return value.GetUint64();
            } else {
                return value.GetDouble();
            }
        }
        case rapidjson::kStringType:
            return value.GetString();
        case rapidjson::kArrayType: {
            dynamic_t::array_t array;
            for (auto it = value.Begin(); it != value.End(); ++it) {
                array.push_back(transformer_t<value_type>::transform(*it));
            }
            return array;
        }
        case rapidjson::kObjectType: {
            dynamic_t::object_t object;
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
                std::string name = it->name.GetString();
                dynamic_t value = transformer_t<value_type>::transform(it->value);
                object[name] = value;
            }
            return object;
        }
        default:
            BOOST_ASSERT(false);
        }
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
