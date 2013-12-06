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
            const std::string& name = *it;
            if (boost::starts_with(name, string::VARIADIC_KEY_PREFIX)) {
                handle_variadic(name, attributes, &fmt);
                continue;
            }

            auto ait = attributes.find(name);
            if (ait == attributes.end()) {
                throw error_t("bad format string '%s' - key '%s' was not provided", m_config.pattern, name);
            }
            const log::attribute_t& attribute = ait->second;
            fmt % attribute.value;
        }
        return fmt.str();
    }

private:
    inline void handle_variadic(const std::string& name, const log::attributes_t& attributes, boost::format* fmt) const {
        for (auto it = name.begin() + string::VARIADIC_KEY_PRFFIX_LENGTH; it != name.end(); ++it) {
            const char ch = *it;
            const std::uint32_t num = ch - '0';
            const log::attribute_t::type_t type = static_cast<log::attribute_t::type_t>(num);
            std::stringstream buf;
            for (auto ait = attributes.begin(); ait != attributes.end(); ++ait) {
                if (ait != attributes.begin()) {
                    buf << ", ";
                }

                const log::attribute_t& attribute = ait->second;
                if (attribute.type == type) {
                    buf << "'" << ait->first << "': '" << attribute.value << "'";
                }
            }

            (*fmt) % buf.str();
        }
    }
};

} // namespace formatter

} // namespace blackhole
