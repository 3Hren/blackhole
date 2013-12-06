#pragma once

#include "config.hpp"

namespace blackhole {

namespace formatter {

namespace string {

struct pattern_parser_t {
    static config_t parse(const std::string& pattern) {
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
                        switch (ch) {
                        case 'L':
                            *it = '0';
                            break;
                        default:
                            *it = '0';
                        }
                    }
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

} // namespace string

} // namespace formatter

} // namespace blackhole
