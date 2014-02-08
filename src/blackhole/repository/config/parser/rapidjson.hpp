#pragma once

#include <rapidjson/document.h>

#include "blackhole/repository/config/base.hpp"
#include "blackhole/repository/config/log.hpp"
#include "blackhole/repository/config/parser.hpp"
#include "blackhole/repository/config/conversion/convert.hpp"

namespace blackhole {

namespace repository {

namespace config {

namespace aux {

template<typename T>
static void fill(T& builder, const rapidjson::Value& node, const std::string& path) {
    for (auto it = node.MemberBegin(); it != node.MemberEnd(); ++it) {
        const std::string& name = it->name.GetString();
        const rapidjson::Value& value = it->value;

        if (value.IsObject()) {
            auto nested_builder = builder[name];
            fill(nested_builder, value, path + "/" + name);
        } else if (value.IsArray() || value.IsNull()) {
            throw blackhole::error_t("array and null parsing is not implemented yet");
        } else {
            if (name == "type") {
                continue;
            }

            if (value.IsBool()) {
                builder[name] = value.GetBool();
            } else if (value.IsDouble()) {
                builder[name] = value.GetDouble();
            } else if (value.IsInt()) {
                conversion::aux::convert(builder, name, path, value.GetInt());
            } else if (value.IsUint()) {
                conversion::aux::convert(builder, name, path, value.GetUint());
            } else if (value.IsInt64()) {
                conversion::aux::convert(builder, name, path, value.GetInt64());
            } else if (value.IsUint64()) {
                conversion::aux::convert(builder, name, path, value.GetUint64());
            } else if (value.IsString()) {
                builder[name] = std::string(value.GetString());
            }
        }
    }
}

} // namespace aux

template<>
class parser_t<repository::config::base_t> {
public:
    template<typename T>
    static T parse(const std::string& path, const rapidjson::Value& value) {
        const std::string& type = value["type"].GetString();
        T config(type);
        aux::fill(config, value, path + "/" + type);
        return config;
    }
};

template<>
class parser_t<formatter_config_t> {
public:
    static formatter_config_t parse(const rapidjson::Value& value) {
        return parser_t<repository::config::base_t>::parse<formatter_config_t>("formatter", value);
    }
};

template<>
class parser_t<sink_config_t> {
public:
    static sink_config_t parse(const rapidjson::Value& value) {
        return parser_t<repository::config::base_t>::parse<sink_config_t>("sink", value);
    }
};

template<>
class parser_t<frontend_config_t> {
public:
    static frontend_config_t parse(const rapidjson::Value& value) {
        if (!(value.HasMember("formatter") && value.HasMember("sink"))) {
            throw blackhole::error_t("both 'formatter' and 'sink' section must be specified");
        }

        const formatter_config_t& formatter = parser_t<formatter_config_t>::parse(value["formatter"]);
        const sink_config_t& sink = parser_t<sink_config_t>::parse(value["sink"]);
        return frontend_config_t { formatter, sink };
    }
};

template<>
class parser_t<log_config_t> {
public:
    static log_config_t parse(const std::string& name, const rapidjson::Value& value) {
        log_config_t config;
        config.name = name;
        for (auto it = value.Begin(); it != value.End(); ++it) {
            const frontend_config_t& frontend = parser_t<frontend_config_t>::parse(*it);
            config.frontends.push_back(frontend);
        }

        return config;
    }
};

template<>
class parser_t<std::vector<log_config_t>> {
public:
    static std::vector<log_config_t> parse(const rapidjson::Value& root) {
        std::vector<log_config_t> configs;
        for (auto it = root.MemberBegin(); it != root.MemberEnd(); ++it) {
            const log_config_t& config = parser_t<log_config_t>::parse(it->name.GetString(), it->value);
            configs.push_back(config);
        }

        return configs;
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
