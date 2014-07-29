#pragma once

#include <string>
#include <vector>

#include "blackhole/error.hpp"
#include "blackhole/forwards.hpp"
#include "blackhole/repository/config/base.hpp"
#include "blackhole/repository/config/log.hpp"

namespace blackhole {

namespace repository {

namespace config {

template<>
class parser_t<repository::config::base_t> {
public:
    template<class T>
    static
    typename std::enable_if<
        std::is_base_of<base_t, T>::value,
        T
    >::type
    parse(std::string component, dynamic_t value) {
        if (!value.contains("type")) {
            //!@todo: Throw specialized exception.
            throw blackhole::error_t("'type' field is missing for '%s' component", component);
        }

        std::string type = value["type"].to<std::string>();
        T config(std::move(type));
        config.config(std::move(value.to<dynamic_t::object_t>()));
        return config;
    }
};

template<>
class parser_t<formatter_config_t> {
public:
    static formatter_config_t parse(dynamic_t value) {
        return parser_t<
            repository::config::base_t
        >::parse<formatter_config_t>("formatter", std::move(value));
    }
};

template<>
class parser_t<sink_config_t> {
public:
    static sink_config_t parse(dynamic_t value) {
        return parser_t<
            repository::config::base_t
        >::parse<sink_config_t>("sink", std::move(value));
    }
};

template<>
class parser_t<frontend_config_t> {
public:
    static frontend_config_t parse(dynamic_t value) {
        if (!(value.contains("formatter") && value.contains("sink"))) {
            //!@todo: Throw specialized exception.
            throw blackhole::error_t("both 'formatter' and 'sink' sections must be specified");
        }

        auto form = parser_t<formatter_config_t>::parse(value["formatter"]);
        auto sink = parser_t<sink_config_t>::parse(value["sink"]);
        return frontend_config_t { std::move(form), std::move(sink) };
    }
};

template<>
class parser_t<log_config_t> {
public:
    static log_config_t parse(std::string name, dynamic_t value) {
        log_config_t config;
        config.name = std::move(name);

        auto array = value.to<dynamic_t::array_t>();
        for (auto it = array.begin(); it != array.end(); ++it) {
            auto frontend = parser_t<frontend_config_t>::parse(*it);
            config.frontends.push_back(std::move(frontend));
        }

        return config;
    }
};

template<>
class parser_t<std::vector<log_config_t>> {
public:
    static std::vector<log_config_t> parse(dynamic_t root) {
        std::vector<log_config_t> configs;
        auto object = root.to<dynamic_t::object_t>();
        for (auto it = object.begin(); it != object.end(); ++it) {
            auto config = parser_t<
                log_config_t
            >::parse(it->first, it->second);
            configs.push_back(std::move(config));
        }

        return configs;
    }
};

namespace parser {

template<class From, class To>
class adapter_t {
public:
    static To parse(const From& value) {
        return parser_t<To>::parse(
            transformer_t<From>::transform(value)
        );
    }
};

template<class From>
class adapter_t<From, log_config_t> {
    typedef log_config_t result_type;

public:
    static result_type parse(std::string name, const From& value) {
        return parser_t<result_type>::parse(
            std::move(name),
            transformer_t<From>::transform(value)
        );
    }
};

} // namespace parser

} // namespace config

} // namespace repository

} // namespace blackhole
