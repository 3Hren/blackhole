#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/error.hpp"
#include "blackhole/formatter/string/parser.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

class mapper_t {
public:
    typedef std::function<std::string(const log::attribute_value_t&)> handler_type;

    void add_handler(const std::string& key, handler_type handler) {
        m_handlers[key] = handler;
    }

    void handle(const std::string& key, const log::attribute_value_t& value, boost::format* fmt) const {
        auto it = m_handlers.find(key);
        if (it != m_handlers.end()) {
            const handler_type& handler = it->second;
            (*fmt) % handler(value);
        } else {
            (*fmt) % value;
        }
    }

private:
    std::unordered_map<std::string, handler_type> m_handlers;
};

class string_t {
    const string::config_t m_config;
    mapper_t m_mapper;

public:
    string_t(const std::string& pattern) :
        m_config(string::pattern_parser_t::parse(pattern))
    {
    }

    void add_mapper(const std::string& key, mapper_t::handler_type mapper) {
        m_mapper.add_handler(key, mapper);
    }

    std::string format(const log::record_t& record) const {
        boost::format fmt(m_config.pattern);
        const log::attributes_t& attributes = record.attributes;
        const std::vector<std::string>& names = m_config.attribute_names;

        for (auto it = names.begin(); it != names.end(); ++it) {
            const std::string& key = *it;
            format_key(key, attributes, &fmt);
        }
        return fmt.str();
    }

private:
    inline void format_key(const std::string& key, const log::attributes_t& attributes, boost::format* fmt) const {
        if (boost::starts_with(key, string::VARIADIC_KEY_PREFIX)) {
            format_variadic_key(key, attributes, fmt);
        } else {
            format_single_key(key, attributes, fmt);
        }
    }

    inline void format_single_key(const std::string& key, const log::attributes_t& attributes, boost::format* fmt) const {
        const log::attribute_t& attribute = get_attribute(attributes, key);
        m_mapper.handle(key, attribute.value, fmt);
    }

    inline void format_variadic_key(const std::string& key, const log::attributes_t& attributes, boost::format* fmt) const {
        typedef log::attribute::scope_underlying_type scope_underlying_type;

        for (auto it = key.begin() + string::VARIADIC_KEY_PRFFIX_LENGTH; it != key.end(); ++it) {
            const scope_underlying_type scope = *it - '0';
            std::vector<std::string> formatted;
            for (auto attr_it = attributes.begin(); attr_it != attributes.end(); ++attr_it) {
                const std::string& name = attr_it->first;
                const log::attribute_t& attribute = attr_it->second;
                if (static_cast<scope_underlying_type>(attribute.scope) & scope) {
                    std::stringstream buffer;
                    buffer << "'" << name << "': '" << attribute.value << "'";
                    formatted.push_back(buffer.str());
                }
            }

            (*fmt) % boost::join(formatted, ", ");
        }
    }

    log::attribute_t get_attribute(const log::attributes_t& attributes, const std::string& key) const {
        auto it = attributes.find(key);
        if (it == attributes.end()) {
            throw error_t("bad format string '%s' - key '%s' was not provided", m_config.pattern, key);
        }
        return it->second;
    }
};

} // namespace formatter

} // namespace blackhole
