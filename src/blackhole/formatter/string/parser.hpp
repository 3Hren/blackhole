#pragma once

#include <boost/algorithm/string.hpp>

#include "config.hpp"

namespace blackhole {

namespace formatter {

namespace string {

static const char VARIADIC_KEY_PREFIX[] = "...";

struct pattern_parser_t {
    static config_t parse(const std::string& input_pattern) {
        auto current = input_pattern.begin();
        auto end = input_pattern.end();

        std::string pattern;
        pattern.reserve(input_pattern.length());
        std::vector<std::string> keys;

        while(current != end) {
            if (begin_key(current, end)) {
                pattern.push_back('%');
                keys.push_back(extract_key(current, end));
            } else {
                pattern.push_back(*current);
            }
            current++;
        }

        return { pattern, keys };
    }

private:
    static inline bool begin_key(std::string::const_iterator it, std::string::const_iterator end) {
        return (*it == '%') && (it + 1 != end) && (*(it + 1) == '(');
    }

    static inline bool end_key(std::string::const_iterator it, std::string::const_iterator end) {
        return (*it == ')') && (it + 1 != end) && (*(it + 1) == 's');
    }

    static inline std::string extract_key(std::string::const_iterator& it, std::string::const_iterator end) {
        it += 2;
        std::string key;
        while (it != end) {
            if (end_key(it, end)) {
                break;
            } else {
                key.push_back(*it);
            }
            it++;
        }

        if (boost::starts_with(key, VARIADIC_KEY_PREFIX)) {
            handle_variadic_key(&key);
        }

        return key;
    }

    static void handle_variadic_key(std::string* key) {
        for (auto it = key->begin() + 3; it != key->end(); ++it) {
            char ch = *it;
            switch (ch) {
            case 'L':
                *it = '0';
                break;
            default:
                *it = '0';
            }
        }
    }
};

} // namespace string

} // namespace formatter

} // namespace blackhole
