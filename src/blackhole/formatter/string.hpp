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
    const string::config_t m_config;

public:
    string_t(const std::string& pattern) :
        m_config(string::pattern_parser_t::parse(pattern))
    {
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
            fmt % attribute.value;
        }
        return fmt.str();
    }

private:
    inline void handle_variadic(const std::string& key, const log::attributes_t& attributes, boost::format* fmt) const {
        for (auto it = key.begin() + string::VARIADIC_KEY_PRFFIX_LENGTH; it != key.end(); ++it) {
            const std::uint32_t num = *it - '0';
            const log::attribute_t::type_t type = static_cast<log::attribute_t::type_t>(num);
            std::vector<std::string> formatted;
            for (auto attr_it = attributes.begin(); attr_it != attributes.end(); ++attr_it) {
                const log::attribute_t& attribute = attr_it->second;
                if (attribute.type == type) {
                    std::stringstream buffer;
                    buffer << "'" << attr_it->first << "': '" << attribute.value << "'";
                    formatted.push_back(buffer.str());
                }
            }

            (*fmt) % boost::join(formatted, ", ");
        }
    }
};

} // namespace formatter

} // namespace blackhole
