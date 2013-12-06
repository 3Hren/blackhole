#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>

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
            //if name.startswith("...")
            //  auto sit = name.begin() + 3;
            //  for (; sit != name.end(); ++sit)
            //      attr_level_t level = to_int(*sit); // sit - число в строковом представлении
            //      str = ""
            //      for (name, (type, value) in attributes(level))
            //          if type == attr_level
            //              fmt = boost::format("%s = %s") % name % value;
            //              str += fmt.str()
            //              if !attr.last()?
            //                  str += ", "
            //
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
