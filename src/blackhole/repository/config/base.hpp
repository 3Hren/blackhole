#pragma once

#include <map>
#include <string>

#include <boost/any.hpp>

#include "blackhole/error.hpp"
#include "blackhole/repository/factory/traits/cast.hpp"

namespace blackhole {

namespace repository {

namespace config {

typedef boost::any holder_type;
typedef std::vector<holder_type> array_type;
typedef std::map<std::string, holder_type> map_type;

//!@todo: Write documentation.
struct base_t {
    std::string type;
    holder_type config;

    base_t(std::string type) :
        type(std::move(type)),
        config(map_type())
    {}

    template<typename T>
    base_t& operator=(const T& value) {
        config = value;
        return *this;
    }

    //!@todo: Merge these two classes into the single one.
    struct builder_t {
        holder_type& any;

        template<typename T>
        builder_t& operator=(const T& value) {
            any = value;
            return *this;
        }

        builder_t& operator=(const char* value) {
            return operator=(std::string(value));
        }

        builder_t operator[](array_type::size_type id) {
            auto* array = boost::any_cast<array_type>(&any);
            if (!array) {
                any = array_type();
                array = boost::any_cast<array_type>(&any);
            }
            if (id >= array->size()) {
                array->resize(id + 1);
            }
            return builder_t { array->at(id) };
        }

        builder_t operator[](const std::string& name) {
            auto* map = boost::any_cast<map_type>(&any);
            if (!map) {
                any = map_type();
                map = boost::any_cast<map_type>(&any);
            }

            if (map->find(name) == map->end()) {
                map->insert(std::make_pair(name, holder_type()));
            }

            return builder_t { map->at(name) };
        }
    };

    struct extractor_t {
        const boost::any& any;

        extractor_t operator [](std::vector<boost::any>::size_type id) const {
            auto* array = boost::any_cast<std::vector<boost::any>>(&any);
            if (!array) {
                throw blackhole::error_t("not array");
            }

            return extractor_t { array->at(id) };
        }

        extractor_t operator [](const std::string& name) const {
            auto* map = boost::any_cast<std::map<std::string, boost::any>>(&any);
            if (!map) {
                throw blackhole::error_t("not map");
            }

            return extractor_t { map->at(name) };
        }

        template<typename T>
        void to(T& value) const {
            try {
                aux::any_to(any, value);
            } catch (boost::bad_any_cast&) {
                throw error_t("conversion error");
            }
        }

        template<typename T>
        T to() const {
            T value;
            to(value);
            return value;
        }
    };

    builder_t operator[](array_type::size_type id) {
        auto& array = *boost::any_cast<std::vector<boost::any>>(&config);
        return builder_t { array[id] };
    }

    builder_t operator[](const std::string& name) {
        auto& map = *boost::any_cast<std::map<std::string, boost::any>>(&config);
        return builder_t { map[name] };
    }

    extractor_t operator[](array_type::size_type id) const {
        const auto& array = *boost::any_cast<std::vector<boost::any>>(&config);
        return extractor_t { array.at(id) };
    }

    extractor_t operator[](const std::string& name) const {
        const auto& map = *boost::any_cast<std::map<std::string, boost::any>>(&config);
        return extractor_t { map.at(name) };
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
