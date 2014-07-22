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
            std::cout << "null" << std::endl;
            throw blackhole::error_t("null values are not supported");
        case rapidjson::kFalseType:
        case rapidjson::kTrueType:
            return value.GetBool();
        case rapidjson::kNumberType: {
            if (value.IsInt()) {
                std::cout << "number: " << value.GetInt() << std::endl;
                return value.GetInt();
            } else if (value.IsInt64()) {
                std::cout << "number: " << value.GetInt64() << std::endl;
                return value.GetInt64();
            } else if (value.IsUint()) {
                std::cout << "number: " << value.GetUint() << std::endl;
                return value.GetUint();
            } else if (value.IsUint64()) {
                std::cout << "number: " << value.GetUint64() << std::endl;
                return value.GetUint64();
            } else {
                std::cout << "number: " << value.GetDouble() << std::endl;
                return value.GetDouble();
            }
        }
        case rapidjson::kStringType:
            std::cout << "string: " << value.GetString() << std::endl;
            return value.GetString();
        case rapidjson::kArrayType: {
            std::cout << "begin array" << std::endl;
            dynamic_t::array_t array;
            for (auto it = value.Begin(); it != value.End(); ++it) {
                array.push_back(transformer_t<value_type>::transform(*it));
            }
            std::cout << "end array" << std::endl;
            return array;
        }
        case rapidjson::kObjectType: {
            std::cout << "begin object" << std::endl;
            dynamic_t::object_t object;
            for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
                std::string name = it->name.GetString();
                dynamic_t value = transformer_t<value_type>::transform(it->value);
                std::cout << "key: " << name << std::endl;
                object[name] = value;
            }
            std::cout << "end object" << std::endl;
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
