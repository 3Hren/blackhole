#pragma once

#include <string>

#include <boost/any.hpp>

namespace blackhole {

namespace repository {

namespace config {

struct base_t {
    std::string type;
    boost::any config;

    base_t(const std::string& type) :
        type(type)
    {
        config = std::map<std::string, boost::any>();
    }

    struct builder_t {
        boost::any& any;

        template<typename T>
        builder_t& operator =(const T& value) {
            any = value;
            return *this;
        }

        builder_t& operator =(const char* value) {
            return operator =(std::string(value));
        }

        builder_t operator [](const std::string& name) {
            auto* map = boost::any_cast<std::map<std::string, boost::any>>(&any);
            if (!map) {
                any = std::map<std::string, boost::any>();
                map = boost::any_cast<std::map<std::string, boost::any>>(&any);
            }

            return builder_t { map->operator[](name) };
        }
    };

    builder_t operator [](const std::string& name) {
        auto& map = *boost::any_cast<std::map<std::string, boost::any>>(&config);
        return builder_t { map[name] };
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
