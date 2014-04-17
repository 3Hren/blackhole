#pragma once

#include <string>
#include <vector>

#include "blackhole/repository/config/base.hpp"
#include "blackhole/repository/config/log.hpp"

namespace blackhole {

namespace repository {

namespace config {

template<typename T>
struct convert_adapter;

template<class T>
struct filler;

template<class From, class To>
class parser_t;

template<class From>
class parser_t<From, repository::config::base_t> {
    typedef convert_adapter<From> conv;

public:
    template<typename T>
    static T parse(const std::string& path, const From& value) {
        auto type = conv::as_string(value["type"]);
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
    typedef convert_adapter<From> conv;

public:
    static frontend_config_t parse(const From& value) {
        if (!(conv::has("formatter", value) && conv::has("sink", value))) {
            throw blackhole::error_t("both 'formatter' and 'sink' section must be specified");
        }

        const auto& formatter = parser_t<From, formatter_config_t>::parse(value["formatter"]);
        const auto& sink = parser_t<From, sink_config_t>::parse(value["sink"]);
        return frontend_config_t { formatter, sink };
    }
};

template<class From>
class parser_t<From, log_config_t> {
    typedef convert_adapter<From> conv;

public:
    static log_config_t parse(const std::string& name, const From& value) {
        log_config_t config;
        config.name = name;
        for (auto it = conv::begin(value); it != conv::end(value); ++it) {
            const auto& frontend = parser_t<From, frontend_config_t>::parse(*it);
            config.frontends.push_back(frontend);
        }

        return config;
    }
};

template<class From>
class parser_t<From, std::vector<log_config_t>> {
    typedef convert_adapter<From> conv;

public:
    static std::vector<log_config_t> parse(const From& root) {
        std::vector<log_config_t> configs;
        for (auto it = conv::object_begin(root); it != conv::object_end(root); ++it) {
            const auto& config = parser_t<From, log_config_t>::parse(conv::name(it), conv::value(it));
            configs.push_back(config);
        }

        return configs;
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
