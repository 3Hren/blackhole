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
    typedef adapter::array_traits<From> at;
    typedef adapter::object_traits<From> ot;

public:
    template<typename T>
    static T parse(const std::string& path, const From& value) {
        const auto* type_member = ot::at(value, "type");
        if (!type_member) {
            throw blackhole::error_t("'type' field if missing for %s", path);
        }

        std::string type = ot::as_string(*type_member);
        T config(type);
        filler<From>::fill(config, value, path + "/" + type);
        return config;
    }
};

template<class From>
class parser_t<From, formatter_config_t> {
public:
    static formatter_config_t parse(const From& value) {
        return parser_t<From, repository::config::base_t>::template parse<formatter_config_t>("formatter", value);
    }
};

template<class From>
class parser_t<From, sink_config_t> {
public:
    static sink_config_t parse(const From& value) {
        return parser_t<From, repository::config::base_t>::template parse<sink_config_t>("sink", value);
    }
};

template<class From>
class parser_t<From, frontend_config_t> {
    typedef adapter::object_traits<From> ot;

public:
    static frontend_config_t parse(const From& value) {
        const auto* form_member = ot::at(value, "formatter");
        const auto* sink_member = ot::at(value, "sink");
        if (!(form_member && sink_member)) {
            throw blackhole::error_t("both 'formatter' and 'sink' sections must be specified");
        }

        const auto& form = parser_t<From, formatter_config_t>::parse(*form_member);
        const auto& sink = parser_t<From, sink_config_t>::parse(*sink_member);
        return frontend_config_t { form, sink };
    }
};

template<class From>
class parser_t<From, log_config_t> {
    typedef adapter::array_traits<From> at;

public:
    static log_config_t parse(const std::string& name, const From& value) {
        log_config_t config;
        config.name = name;
        for (auto it = at::begin(value); it != at::end(value); ++it) {
            const auto& frontend = parser_t<From, frontend_config_t>::parse(*it);
            config.frontends.push_back(frontend);
        }

        return config;
    }
};

template<class From>
class parser_t<From, std::vector<log_config_t>> {
    typedef adapter::object_traits<From> ot;

public:
    static std::vector<log_config_t> parse(const From& root) {
        std::vector<log_config_t> configs;
        for (auto it = ot::begin(root); it != ot::end(root); ++it) {
            const auto& config = parser_t<From, log_config_t>::parse(ot::name(it), ot::value(it)); //!@todo: own iterator
            configs.push_back(config);
        }

        return configs;
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
