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
    typedef adapter::array_traits<rapidjson::Value> array;
    typedef adapter::object_traits<rapidjson::Value> object;

    template<typename T>
    static void fill(T& builder, const rapidjson::Value& value) {
        switch (value.GetType()) {
        case rapidjson::kNullType:
            std::cout << "null" << std::endl;
            throw blackhole::error_t("null values are not supported");
            break;
        case rapidjson::kFalseType:
        case rapidjson::kTrueType:
            std::cout << "bool: " << value.GetBool() << std::endl;
            builder = value.GetBool();
            break;
        case rapidjson::kNumberType: {
            if (value.IsInt()) {
                std::cout << "number: " << value.GetInt() << std::endl;
                builder = value.GetInt();
            } else if (value.IsInt64()) {
                std::cout << "number: " << value.GetInt64() << std::endl;
                builder = value.GetInt64();
            } else if (value.IsUint()) {
                std::cout << "number: " << value.GetUint() << std::endl;
                builder = value.GetUint();
            } else if (value.IsUint64()) {
                std::cout << "number: " << value.GetUint64() << std::endl;
                builder = value.GetUint64();
            } else {
                std::cout << "number: " << value.GetDouble() << std::endl;
                builder = value.GetDouble();
            }
            break;
        }
        case rapidjson::kStringType:
            std::cout << "string: " << value.GetString() << std::endl;
            builder = value.GetString();
            break;
        case rapidjson::kArrayType:
            std::cout << "begin array" << std::endl;
            for (auto it = array::begin(value); it != array::end(value); ++it) {
                auto pos = std::distance(array::begin(value), it);
                auto recursive = builder[pos];
                filler<rapidjson::Value>::fill(recursive, *it);
            }
            std::cout << "end array" << std::endl;
            break;
        case rapidjson::kObjectType: {
            std::cout << "begin object" << std::endl;
            for (auto it = object::begin(value); it != object::end(value); ++it) {
                const auto& name = object::name(it);
                const auto& value = object::value(it);
                std::cout << "key: " << name << std::endl;
                auto recursive = builder[name];
                filler<rapidjson::Value>::fill(recursive, value);
            }
            std::cout << "end object" << std::endl;
            break;
        }
        default:
            BOOST_ASSERT(false);
        }
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
