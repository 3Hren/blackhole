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

class string_t {
    typedef std::function<std::string(const log::attribute_value_t&)> mapper_t;
    const string::config_t m_config;
    std::unordered_map<std::string, mapper_t> m_mappers;
public:
    string_t(const std::string& pattern) :
        m_config(string::pattern_parser_t::parse(pattern))
    {
    }

    void set_mapper(const std::string& key, mapper_t mapper) {
        m_mappers[key] = mapper;
    }

    std::string format(const log::record_t& record) const {
        boost::format fmt(m_config.pattern);
        const log::attributes_t& attributes = record.attributes;
        const std::vector<std::string>& names = m_config.attribute_names;

        for (auto it = names.begin(); it != names.end(); ++it) {
            const std::string& key = *it;
            if (boost::starts_with(key, string::VARIADIC_KEY_PREFIX)) {
                handle_variadic(key, attributes, &fmt);
                continue;
            }

            auto ait = attributes.find(key);
            if (ait == attributes.end()) {
                throw error_t("bad format string '%s' - key '%s' was not provided", m_config.pattern, key);
            }
            const log::attribute_t& attribute = ait->second;
            {
                auto it = m_mappers.find(key);
            if (it != m_mappers.end()) {
                fmt % it->second(attribute.value);
            } else {
                fmt % attribute.value;
            }
            }
        }
        return fmt.str();
    }

private:
    inline void handle_variadic(const std::string& key, const log::attributes_t& attributes, boost::format* fmt) const {
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
};

} // namespace formatter

} // namespace blackhole
