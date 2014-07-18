#pragma once

#include <string>
#include <vector>

#include "blackhole/repository/config/base.hpp"
#include "blackhole/repository/config/log.hpp"

namespace blackhole {

namespace repository {

namespace config {

namespace adapter {

template<typename T>
struct array_traits;

template<typename T>
struct object_traits;

} // namespace adapter

template<class T>
struct filler;

template<class From, class To>
class parser_t;

template<class From>
class parser_t<From, repository::config::base_t> {
    typedef From from_type;
    typedef adapter::array_traits<from_type> array;
    typedef adapter::object_traits<from_type> object;

public:
    template<class T>
    static
    typename std::enable_if<
        std::is_base_of<base_t, T>::value,
        T
    >::type
    parse(std::string component, const from_type& value) {
        if (!object::has(value, "type")) {
            throw blackhole::error_t("'type' field if missing for '%s' component", component);
        }

        std::string type = object::as_string(object::at(value, "type"));
        T config(std::move(type));
        filler<from_type>::fill(static_cast<base_t&>(config), value);
        return config;
    }
};

template<class From>
class parser_t<From, formatter_config_t> {
    typedef From from_type;

public:
    static formatter_config_t parse(const from_type& value) {
        return parser_t<
            from_type,
            repository::config::base_t
        >::template parse<formatter_config_t>("formatter", value);
    }
};

template<class From>
class parser_t<From, sink_config_t> {
    typedef From from_type;

public:
    static sink_config_t parse(const from_type& value) {
        return parser_t<
            from_type,
            repository::config::base_t
        >::template parse<sink_config_t>("sink", value);
    }
};

template<class From>
class parser_t<From, frontend_config_t> {
    typedef From from_type;
    typedef adapter::object_traits<from_type> object;

public:
    static frontend_config_t parse(const from_type& value) {
        if (!(object::has(value, "formatter") && object::has(value, "sink"))) {
            throw blackhole::error_t("both 'formatter' and 'sink' sections must be specified");
        }

        auto form = parser_t<from_type, formatter_config_t>::parse(object::at(value, "formatter"));
        auto sink = parser_t<from_type, sink_config_t>::parse(object::at(value, "sink"));
        return frontend_config_t { std::move(form), std::move(sink) };
    }
};

template<class From>
class parser_t<From, log_config_t> {
    typedef From from_type;
    typedef adapter::array_traits<from_type> array;

public:
    static log_config_t parse(const std::string& name, const from_type& value) {
        log_config_t config;
        config.name = name;
        for (auto it = array::begin(value); it != array::end(value); ++it) {
            auto frontend = parser_t<from_type, frontend_config_t>::parse(*it);
            config.frontends.push_back(std::move(frontend));
        }

        return config;
    }
};

template<class From>
class parser_t<From, std::vector<log_config_t>> {
    typedef From from_type;
    typedef adapter::object_traits<from_type> object;

public:
    static std::vector<log_config_t> parse(const from_type& root) {
        std::vector<log_config_t> configs;
        for (auto it = object::begin(root); it != object::end(root); ++it) {
            auto config = parser_t<
                from_type,
                log_config_t
            >::parse(object::name(it), object::value(it));
            configs.push_back(std::move(config));
        }

        return configs;
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
