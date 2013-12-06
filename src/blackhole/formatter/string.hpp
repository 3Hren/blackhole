#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/error.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

class string_t {
    struct config_t {
        std::string pattern;
        std::vector<std::string> attribute_names;
    };

    const config_t m_config;
public:
    string_t(const std::string& pattern) :
        m_config(init(pattern))
    {
    }

    std::string
    format(const log::record_t& record) const {
        boost::format fmt(m_config.pattern);
        const log::attributes_t& attributes = record.attributes;
        const std::vector<std::string>& names = m_config.attribute_names;

        for (auto it = names.begin(); it != names.end(); ++it) {
            const std::string& name = *it;
            if (boost::starts_with(name, "...")) {
                for (auto sit = name.begin() + 3; sit != name.end(); ++sit) {
                    char ch = *sit;
                    std::uint32_t num = ch - '0';
                    log::attribute_t::type_t type = static_cast<log::attribute_t::type_t>(num);
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
                    fmt % buf.str();
                }
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
    static
    config_t
    init(const std::string& pattern) {
        auto current = pattern.begin();
        auto end = pattern.end();

        std::string fpattern;
        fpattern.reserve(pattern.length());
        std::vector<std::string> attribute_names;

        while(current != end) {
            if ((*current == '%') && (current + 1 != end) && (*(current + 1) == '(')) {
                fpattern.push_back('%');
                current += 2;
                std::string key;
                while (current != end) {
                    if ((*current == ')') && (current + 1 != end) && (*(current + 1) == 's')) {
                        break;
                    } else {
                        key.push_back(*current);
                    }
                    current++;
                }
                if (boost::starts_with(key, "...")) {
                    for (auto it = key.begin() + 3; it != key.end(); ++it) {
                        char ch = *it;
                        std::cout << ch << std::endl;
                        switch (ch) {
                        case 'L':
                            *it = '0';
                            std::cout << *it << std::endl;
                            break;
                        default:
                            *it = '0';
                        }
                    }
                }
                std::cout << key << std::endl;
                attribute_names.push_back(key);
            } else {
                fpattern.push_back(*current);
            }
            current++;
        }

        return { fpattern, attribute_names };
    }
};

} // namespace formatter

} // namespace blackhole
