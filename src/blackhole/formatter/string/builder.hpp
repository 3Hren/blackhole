#pragma once

#include <functional>

#include <boost/algorithm/string.hpp>

#include "builder/literal.hpp"
#include "builder/placeholder.hpp"
#include "builder/variadic.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

typedef std::function<
    void(blackhole::aux::attachable_ostringstream&,
         const mapping::value_t&,
         const log::attributes_t&)
> type;

} // namespace builder

namespace aux {

//! Simple factory function that maps scope character to its enumeration type.
inline log::attribute::scope map_to_scope(const char ch) {
    switch (ch) {
    case 'L':
        return log::attribute::scope::local;
    case 'E':
        return log::attribute::scope::event;
    case 'G':
        return log::attribute::scope::global;
    case 'T':
        return log::attribute::scope::thread;
    case 'U':
        return log::attribute::scope::universe;
    }
    return log::attribute::scope::local;
}

inline void handle_variadic_key(std::string* key) {
    typedef log::attribute::scope_underlying_type scope_underlying_type;

    scope_underlying_type result = 0;
    for (auto it = key->begin() + VARIADIC_KEY_PRFFIX_LENGTH; it != key->end(); ++it) {
        const char ch = *it;
        const log::attribute::scope scope = map_to_scope(ch);
        result |= static_cast<scope_underlying_type>(scope);
    }
    // Explicit cast to long long resolves GCC 4.4 ambiguity bug.
    *key = std::string("...") + std::to_string(static_cast<long long>(result));
}

inline bool begin_key(std::string::const_iterator it, std::string::const_iterator end) {
    return (*it == '%') && (it + 1 != end) && (*(it + 1) == '(');
}

inline bool end_key(std::string::const_iterator it, std::string::const_iterator end) {
    return (*it == ')') && (it + 1 != end) && (*(it + 1) == 's');
}

inline std::string extract_key(std::string::const_iterator& it, std::string::const_iterator end) {
    it += 2;
    std::string key;
    while (it != end) {
        if (end_key(it, end)) {
            it++;
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

} // namespace aux

struct formatter_builder_t {
    static std::vector<builder::type> build(const std::string& pattern) {
        auto current = pattern.begin();
        auto end = pattern.end();

        std::vector<builder::type> formatters;

        std::string literal;
        while(current != end) {
            if (aux::begin_key(current, end)) {
                if (!literal.empty()) {
                    formatters.push_back(builder::literal_t{ literal });
                    literal.clear();
                }

                const std::string& key = aux::extract_key(current, end);
                if (boost::starts_with(key, VARIADIC_KEY_PREFIX)) {
                    formatters.push_back(builder::variadic_t{ key });
                } else {
                    formatters.push_back(builder::placeholder_t{ key });
                }
            } else {
                literal.push_back(*current);
            }
            current++;
        }

        if (!literal.empty()) {
            formatters.push_back(builder::literal_t{ literal });
        }

        return formatters;
    }
};

} // namespace string

} // namespace formatter

} // namespace blackhole
