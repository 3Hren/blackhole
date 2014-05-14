#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

#include <rapidjson/document.h>

namespace elasticsearch {

template<class T>
struct extractor_t;

template<>
struct extractor_t<bool> {
    static bool extract(const rapidjson::Value& object) {
        return object.GetBool();
    }
};

template<>
struct extractor_t<long> {
    static long extract(const rapidjson::Value& object) {
        return object.GetInt64();
    }
};

template<>
struct extractor_t<std::string> {
    static std::string extract(const rapidjson::Value& object) {
        return object.GetString();
    }
};

template<typename T>
struct extractor_t<std::vector<T>> {
    static std::vector<T> extract(const rapidjson::Value& object) {
        std::vector<T> vector;
        for (auto it = object.Begin(); it != object.End(); ++it) {
            vector.emplace_back(extractor_t<T>::extract(*it));
        }

        return vector;
    }
};

template<typename T>
struct extractor_t<std::map<std::string, T>> {
    static std::map<std::string, T> extract(const rapidjson::Value& object) {
        std::map<std::string, T> map;
        for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
            map.insert(
                std::make_pair(
                    it->name.GetString(), extractor_t<T>::extract(it->value)
                )
            );
        }

        return map;
    }
};

namespace aux {

template<typename T>
struct field_extractor_t {
    static
    void
    extract(const rapidjson::Value& value, const char* name, T& field) {
        const auto& object = value[name];
        if (object.IsNull()) {
            throw std::runtime_error(
                std::string("required member not found: ") + name
            );
        }

        field = extractor_t<T>::extract(object);
    }
};

template<typename T>
struct field_extractor_t<boost::optional<T>> {
    static
    void
    extract(const rapidjson::Value& value,
            const char* name,
            boost::optional<T>& field) {
        const auto& object = value[name];
        if (!object.IsNull()) {
            field = extractor_t<T>::extract(object);
        }
    }
};

struct extract_helper_t {
    const rapidjson::Value& object;

    extract_helper_t(const rapidjson::Value& value) :
        object(value)
    {}

    template<typename T>
    void to(const char* name, T& field) const {
        field_extractor_t<T>::extract(object, name, field);
    }

    template<typename T, class Predicate, class Map>
    void find(Predicate filtered,
              Map mapped,
              std::map<std::string, T>& field) const {
        for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
            const auto& name = it->name.GetString();
            if (filtered(name)) {
                field[mapped(name)] = extractor_t<T>::extract(it->value);
            }
        }
    }
};

} // namespace aux

namespace filter {

struct endswith_t {
    std::string suffix;

    bool operator()(const std::string& name) const {
        return boost::algorithm::ends_with(name, suffix);
    }
};

} // namespace filter

namespace mapped {

struct substring_t {
    std::string suffix;

    std::string operator()(const std::string& name) const {
        return name.substr(0, name.find(suffix));
    }
};

} // namespace mapped

} // namespace elasticsearch
