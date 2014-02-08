#pragma once

#include <string>

#include <boost/any.hpp>

#include "blackhole/factory.hpp"

namespace blackhole {

namespace repository {

namespace config {

struct base_t {
    std::string type;
    boost::any config;

    base_t(const std::string& type) :
        type(type),
        config(std::map<std::string, boost::any>())
    {
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

            return builder_t { map->operator [](name) };
        }
    };

    struct extractor_t {
        const boost::any& any;

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

    builder_t operator [](const std::string& name) {
        auto& map = *boost::any_cast<std::map<std::string, boost::any>>(&config);
        return builder_t { map[name] };
    }

    extractor_t operator [](const std::string& name) const {
        const auto& map = *boost::any_cast<std::map<std::string, boost::any>>(&config);
        return extractor_t { map.at(name) };
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
