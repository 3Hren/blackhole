#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include "utils/format.hpp"

namespace blackhole {

namespace log {

typedef boost::variant<
    std::uint64_t,
    std::int64_t,
    std::double_t,
    std::string
> attribute_value_t;

typedef std::unordered_map<std::string, attribute_value_t> attributes_t;

struct record_t {
    attributes_t attributes;
};

} // namespace log

class error_t : public std::runtime_error {
public:
    template<typename... Args>
    error_t(const std::string& reason, Args&&... args) :
        std::runtime_error(utils::format(reason, std::forward<Args>(args)...))
    {}
};

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
            auto ait = attributes.find(name);
            if (ait == attributes.end()) {
                throw error_t("bad format string '%s' - key '%s' was not provided", m_config.pattern, name);
            }
            fmt % ait->second;
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

} } // namespace blackhole::formatter
