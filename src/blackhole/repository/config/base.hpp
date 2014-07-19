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

    struct writer_t {
        holder_type& holder;

        template<typename T>
        writer_t& operator=(const T& value) {
            holder = value;
            return *this;
        }

        writer_t& operator=(const char* value) {
            return operator=(std::string(value));
        }

        writer_t operator[](array_type::size_type id) {
            auto array = boost::any_cast<array_type>(&holder);
            if (!array) {
                holder = array_type();
                array = boost::any_cast<array_type>(&holder);
            }
            if (id >= array->size()) {
                array->resize(id + 1);
            }
            return writer_t { array->at(id) };
        }

        writer_t operator[](const std::string& name) {
            auto map = boost::any_cast<map_type>(&holder);
            if (!map) {
                holder = map_type();
                map = boost::any_cast<map_type>(&holder);
            }

            if (map->find(name) == map->end()) {
                map->insert(std::make_pair(name, holder_type()));
            }

            return writer_t { map->at(name) };
        }
    };

    struct reader_t {
        const holder_type& holder;

        reader_t operator[](array_type::size_type id) const {
            auto array = boost::any_cast<array_type>(&holder);
            if (!array) {
                throw blackhole::error_t("not array");
            }

            return reader_t { array->at(id) };
        }

        reader_t operator [](const std::string& name) const {
            auto map = boost::any_cast<map_type>(&holder);
            if (!map) {
                throw blackhole::error_t("not map");
            }

            return reader_t { map->at(name) };
        }

        template<typename T>
        void to(T& value) const {
            try {
                aux::any_to(holder, value);
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

    writer_t operator[](array_type::size_type id) {
        auto& array = *boost::any_cast<array_type>(&config);
        return writer_t { array[id] };
    }

    writer_t operator[](const std::string& name) {
        auto& map = *boost::any_cast<map_type>(&config);
        return writer_t { map[name] };
    }

    reader_t operator[](array_type::size_type id) const {
        const auto& array = *boost::any_cast<array_type>(&config);
        return reader_t { array.at(id) };
    }

    reader_t operator[](const std::string& name) const {
        const auto& map = *boost::any_cast<map_type>(&config);
        return reader_t { map.at(name) };
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
