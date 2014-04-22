#pragma once

#include <rapidjson/document.h>

#include "blackhole/repository/config/parser.hpp"

namespace blackhole {

namespace repository {

namespace config {

// Converter adapter specializations for rapidjson value.
namespace adapter {

template<>
struct array_traits<rapidjson::Value> {
    typedef rapidjson::Value value_type;
    typedef value_type::ConstValueIterator const_iterator;

    static
    const_iterator
    begin(const value_type& value) {
        return value.Begin();
    }

    static
    const_iterator
    end(const value_type& value) {
        return value.End();
    }
};

template<>
struct object_traits<rapidjson::Value> {
    typedef rapidjson::Value value_type;
    typedef value_type::ConstMemberIterator const_iterator;

    static
    const_iterator
    begin(const value_type& value) {
        BOOST_ASSERT(value.IsObject());
        return value.MemberBegin();
    }

    static
    const_iterator
    end(const value_type& value) {
        BOOST_ASSERT(value.IsObject());
        return value.MemberEnd();
    }

    static
    std::string
    name(const const_iterator& it) {
        BOOST_ASSERT(it->name.IsString());
        return std::string(it->name.GetString());
    }

    static
    const value_type&
    value(const const_iterator& it) {
        return it->value;
    }

    static
    bool
    has(const value_type& value, const std::string& name) {
        BOOST_ASSERT(value.IsObject());
        return value.HasMember(name.c_str());
    }

    static
    const value_type&
    at(const value_type& value, const std::string& name) {
        BOOST_ASSERT(has(value, name.c_str()));
        return value[name.c_str()];
    }

    static
    std::string
    as_string(const value_type& value) {
        BOOST_ASSERT(value.IsString());
        return std::string(value.GetString());
    }
};

} // namespace adapter

template<>
struct filler<rapidjson::Value> {
    typedef adapter::object_traits<rapidjson::Value> object;

    template<typename T>
    static void fill(T& builder, const rapidjson::Value& node, const std::string& path) {
        for (auto it = object::begin(node); it != object::end(node); ++it) {
            const auto& name = object::name(it);
            const auto& value = object::value(it);

            if (name == "type") {
                continue;
            }

            switch (value.GetType()) {
            case rapidjson::kNullType:
            case rapidjson::kArrayType:
                throw blackhole::error_t("both null and array parsing are not supported");
                break;
            case rapidjson::kFalseType:
            case rapidjson::kTrueType:
                builder[name] = value.GetBool();
                break;
            case rapidjson::kNumberType: {
                if (value.IsInt()) {
                    builder[name] = value.GetInt();
                } else if (value.IsInt64()) {
                    builder[name] = value.GetInt64();
                } else if (value.IsUint()) {
                    builder[name] = value.GetUint();
                } else if (value.IsUint64()) {
                    builder[name] = value.GetUint64();
                } else {
                    builder[name] = value.GetDouble();
                }
                break;
            }
            case rapidjson::kStringType:
                builder[name] = value.GetString();
                break;
            case rapidjson::kObjectType: {
                auto recursive = builder[name];
                filler<rapidjson::Value>::fill(recursive, value, path + "/" + name);
                break;
            }
            default:
                BOOST_ASSERT(false);
            }
        }
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
