#pragma once

#include <functional>

#include <boost/algorithm/string.hpp>

#include "builder/literal.hpp"
#include "builder/placeholder/required.hpp"
#include "builder/placeholder/optional.hpp"
#include "builder/placeholder/variadic.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

typedef std::function<
    void(blackhole::aux::attachable_ostringstream&,
         const mapping::value_t&,
         const attribute::set_view_t&)
> type;

} // namespace builder

enum placeholder_type_t {
    required,
    optional,
    variadic
};

namespace aux {

//! Simple factory function that maps scope character to its enumeration type.
inline attribute::scope_t map_to_scope(const char ch) {
    switch (ch) {
    case 'L':
        return attribute::scope_t::local;
    case 'E':
        return attribute::scope_t::event;
    case 'G':
        return attribute::scope_t::global;
    case 'T':
        return attribute::scope_t::thread;
    case 'U':
        return attribute::scope_t::universe;
    }
    return attribute::scope_t::local;
}

inline void handle_variadic_key(std::string* key) {
    typedef attribute::scope_underlying_type scope_underlying_type;

    scope_underlying_type result = 0;
    for (auto it = key->begin() + VARIADIC_KEY_PRFFIX_LENGTH; it != key->end(); ++it) {
        const char ch = *it;
        const attribute::scope_t scope = map_to_scope(ch);
        result |= static_cast<scope_underlying_type>(scope);
    }
    // Explicit cast to long long resolves GCC 4.4 ambiguity bug.
    *key = std::string("...") + std::to_string(static_cast<long long>(result));
}

inline
bool
begin_key(std::string::const_iterator it, std::string::const_iterator end) {
    return (*it == '%') && (it + 1 != end) && (*(it + 1) == '(');
}

inline
bool
end_required_key(std::string::const_iterator it, std::string::const_iterator end) {
    return (*it == ')') && (it + 1 != end) && (*(it + 1) == 's');
}

inline
bool
end_optional_key(std::string::const_iterator it, std::string::const_iterator end) {
    return (*it == ')') &&
            (it + 1 != end) && (*(it + 1) == '?') &&
            (it + 2 != end) && (*(it + 2) == 's');
}

inline
std::tuple<std::string, placeholder_type_t>
extract_key(std::string::const_iterator& it, std::string::const_iterator end) {
    it += 2;
    placeholder_type_t type;
    std::string key;
    while (it != end) {
        if (end_required_key(it, end)) {
            it++;
            type = required;
            break;
        } else if (end_optional_key(it, end)) {
            it += 2;
            type = optional;
            break;
        } else {
            key.push_back(*it);
        }

        it++;
    }

    if (boost::starts_with(key, VARIADIC_KEY_PREFIX)) {
        type = variadic;
        handle_variadic_key(&key);
    }

    return std::make_tuple(key, type);
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

                std::string key;
                placeholder_type_t type;
                std::tie(key, type) = aux::extract_key(current, end);
                switch (type) {
                case required:
                    formatters.push_back(builder::placeholder::required_t{ key });
                    break;
                case optional:
                    formatters.push_back(builder::placeholder::optional_t{ key });
                    break;
                case variadic:
                    formatters.push_back(builder::placeholder::variadic_t{ key });
                    break;
                default:
                    BOOST_ASSERT(false);
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
