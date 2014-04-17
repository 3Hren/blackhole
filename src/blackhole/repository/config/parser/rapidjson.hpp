#pragma once

#include <rapidjson/document.h>

#include "blackhole/repository/config/base.hpp"
#include "blackhole/repository/config/log.hpp"
#include "blackhole/repository/config/parser.hpp"

namespace blackhole {

namespace repository {

namespace config {

template<typename T>
struct convert_adapter;

template<>
struct convert_adapter<rapidjson::Value> {
    typedef rapidjson::Value value_type;
    typedef typename rapidjson::Value::ConstValueIterator const_iterator;
    typedef typename rapidjson::Value::ConstMemberIterator const_object_iterator;

    static const_iterator begin(const value_type& value) {
        return value.Begin();
    }

    static const_iterator end(const value_type& value) {
        return value.End();
    }

    static const_object_iterator object_begin(const value_type& value) {
        return value.MemberBegin();
    }

    static const_object_iterator object_end(const value_type& value) {
        return value.MemberEnd();
    }

    static std::string name(const const_object_iterator& it) {
        return std::string(it->name.GetString());
    }

    static const value_type& value(const const_object_iterator& it) {
        return it->value;
    }

    static bool has(const std::string& name, const value_type& value) {
        return value.HasMember(name.c_str());
    }
};

namespace aux {

struct filler {
    typedef rapidjson::Value value_type;
    typedef convert_adapter<value_type> conv;

    template<typename T>
    static void fill(T& builder, const value_type& node, const std::string& path) {
        for (auto it = conv::object_begin(node); it != conv::object_end(node); ++it) {
            const auto& name = conv::name(it);
            const auto& value = conv::value(it);

            if (value.IsObject()) {
                auto recursive = builder[name];
                filler::fill(recursive, value, path + "/" + name);
            } else if (value.IsNull() || value.IsArray()) {
                throw blackhole::error_t("array and null parsing is not implemented yet");
            } else {
                if (name == "type") {
                    continue;
                }

                if (value.IsBool()) {
                    builder[name] = value.GetBool();
                } else if (value.IsInt()) {
                    builder[name] = value.GetInt();
                } else if (value.IsUint()) {
                    builder[name] = value.GetUint();
                } else if (value.IsInt64()) {
                    builder[name] = value.GetInt64();
                } else if (value.IsUint64()) {
                    builder[name] = value.GetUint64();
                } else if (value.IsDouble()) {
                    builder[name] = value.GetDouble();
                } else if (value.IsString()) {
                    builder[name] = std::string(value.GetString());
                }
            }
        }
    }
};

} // namespace aux

template<>
class parser_t<repository::config::base_t> {
    typedef rapidjson::Value value_type;

public:
    template<typename T>
    static T parse(const std::string& path, const value_type& value) {
        const std::string& type = value["type"].GetString();
        T config(type);
        aux::filler::fill(config, value, path + "/" + type);
        return config;
    }
};

template<>
class parser_t<formatter_config_t> {
    typedef rapidjson::Value value_type;

public:
    static formatter_config_t parse(const value_type& value) {
        return parser_t<repository::config::base_t>::parse<formatter_config_t>("formatter", value);
    }
};

template<>
class parser_t<sink_config_t> {
    typedef rapidjson::Value value_type;

public:
    static sink_config_t parse(const value_type& value) {
        return parser_t<repository::config::base_t>::parse<sink_config_t>("sink", value);
    }
};

template<>
class parser_t<frontend_config_t> {
    typedef rapidjson::Value value_type;
    typedef convert_adapter<value_type> conv;

public:
    static frontend_config_t parse(const value_type& value) {
        if (!(conv::has("formatter", value) && conv::has("sink", value))) {
            throw blackhole::error_t("both 'formatter' and 'sink' section must be specified");
        }

        const auto& formatter = parser_t<formatter_config_t>::parse(value["formatter"]);
        const auto& sink = parser_t<sink_config_t>::parse(value["sink"]);
        return frontend_config_t { formatter, sink };
    }
};

template<>
class parser_t<log_config_t> {
    typedef rapidjson::Value value_type;
    typedef convert_adapter<value_type> conv;

public:
    static log_config_t parse(const std::string& name, const value_type& value) {
        log_config_t config;
        config.name = name;
        for (auto it = conv::begin(value); it != conv::end(value); ++it) {
            const auto& frontend = parser_t<frontend_config_t>::parse(*it);
            config.frontends.push_back(frontend);
        }

        return config;
    }
};

template<>
class parser_t<std::vector<log_config_t>> {
    typedef rapidjson::Value value_type;
    typedef convert_adapter<value_type> conv;

public:
    static std::vector<log_config_t> parse(const value_type& root) {
        std::vector<log_config_t> configs;
        for (auto it = conv::object_begin(root); it != conv::object_end(root); ++it) {
            const auto& config = parser_t<log_config_t>::parse(conv::name(it), conv::value(it));
            configs.push_back(config);
        }

        return configs;
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
