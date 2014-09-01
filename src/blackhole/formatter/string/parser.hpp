#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

#include <blackhole/detail/config/noexcept.hpp>

namespace blackhole {

namespace formatter {

namespace string  {

namespace parser {

class error_t : public std::runtime_error {
    const uint pos;
    const std::string inspect;

public:
    error_t(uint pos, const std::string& pattern, const std::string& reason) :
        std::runtime_error("parser error: " + reason),
        pos(pos),
        inspect(
            "parser error: " +  reason + "\n" +
            pattern + "\n" +
            std::string(pos, '~') + "^"
        )
    {}

    ~error_t() BLACKHOLE_NOEXCEPT {}

    std::string detail() const {
        return inspect;
    }
};

class broken_t : public error_t {
public:
    broken_t(uint pos, const std::string& pattern) :
        error_t(pos, pattern, "broken parser")
    {}
};

class exhausted_t : public error_t {
public:
    exhausted_t(uint pos, const std::string& pattern) :
        error_t(pos, pattern, "exhausted state")
    {}
};

class illformed_t : public error_t {
public:
    illformed_t(uint pos, const std::string& pattern) :
        error_t(pos, pattern, "illformed pattern")
    {}
};

class invalid_placeholder_t : public error_t {
public:
    invalid_placeholder_t(uint pos, const std::string& pattern) :
        error_t(pos, pattern, "invalid placeholder name (only [a-zA-Z0-9] allowed)")
    {}
};

}

struct literal_t {
    std::string value;
};

namespace placeholder {

struct required_t {
    std::string name;
};

struct optional_t {
    std::string name;
    std::string prefix;
    std::string suffix;
};

struct variadic_t {
    std::string prefix;
    std::string suffix;
    std::string pattern;
    std::string separator;

    variadic_t() :
        pattern("%k: %v"),
        separator(", ")
    {}
};

}

typedef boost::variant<
    literal_t,
    placeholder::required_t,
    placeholder::optional_t,
    placeholder::variadic_t
> token_t;

static const std::array<char, 2> PH_BEGIN = {{ '%', '(' }};
static const std::array<char, 2> PH_END   = {{ ')', 's' }};
static const std::array<char, 3> PH_VARIADIC = {{ '.', '.', '.' }};

class parser_t {
    enum state_t {
        whatever,
        literal,
        placeholder,
        broken
    };

    std::string pattern;
    std::string::const_iterator pos;
    const std::string::const_iterator begin;
    const std::string::const_iterator end;
    state_t state;

public:
    parser_t(std::string pattern) :
        pattern(std::move(pattern)),
        pos(this->pattern.begin()),
        begin(this->pattern.begin()),
        end(this->pattern.end()),
        state(whatever)
    {}

    token_t
    next() {
        if (state == broken) {
            throw_<parser::broken_t>();
        }

        if (pos == end) {
            throw_<parser::exhausted_t>();
        }

        switch (state) {
        case whatever:
            return parse_whatever();
        case literal:
            return parse_literal();
        case placeholder:
            return parse_placeholder();
        case broken:
            throw_<parser::broken_t>();
        default:
            BOOST_ASSERT(false);
        }
    }

private:
    token_t
    parse_whatever() {
        if (starts_with(pos, end, PH_BEGIN)) {
            pos += PH_BEGIN.size();
            state = placeholder;
        } else {
            state = literal;
        }

        return next();
    }

    literal_t
    parse_literal() {
        literal_t literal;

        while (pos != end) {
            if (starts_with(pos, end, PH_BEGIN)) {
                pos += PH_BEGIN.size();
                state = placeholder;
                return literal;
            }

            literal.value.push_back(*pos);
            pos++;
        }

        return literal;
    }

    token_t
    parse_placeholder() {
        if (starts_with(pos, end, PH_VARIADIC)) {
            pos += PH_VARIADIC.size();
            return parse_variadic();
        }

        std::string name;
        while (pos != end) {
            const char ch = *pos;

            if (std::isalpha(ch) || std::isdigit(ch)) {
                name.push_back(ch);
            } else {
                if (ch == ':') {
                    pos++;
                    return parse_optional(name);
                } else if (starts_with(pos, end, PH_END)) {
                    pos += PH_END.size();
                    state = whatever;
                    return placeholder::required_t { name };
                } else if (ch == ')' && std::next(pos) == end) {
                    throw_<parser::illformed_t>();
                } else {
                    throw_<parser::invalid_placeholder_t>();
                }
            }

            pos++;
        }

        throw_<parser::illformed_t>();
    }

    placeholder::optional_t
    parse_optional(std::string name) {
        placeholder::optional_t ph { std::move(name), "", "" };
        std::tie(ph.prefix, std::ignore) = parse({ ":" });
        std::tie(ph.suffix, std::ignore) = parse({ ")s" });
        state = whatever;
        return ph;
    }

    placeholder::variadic_t
    parse_variadic() {
        if (pos == end) {
            throw_<parser::illformed_t>();
        } else {
            placeholder::variadic_t ph;
            if (*pos == '[') {
                pos++;

                std::tie(ph.pattern, std::ignore) = parse({ "]" });
                if (starts_with(pos, end, PH_END)) {
                    pos += PH_END.size();
                    state = whatever;
                    return ph;
                } else if (*pos == ':') {
                    pos++;
                    parse_variadic_options(ph);
                    state = whatever;
                    return ph;
                } else {
                    throw_<parser::illformed_t>();
                }
            } else if (*pos == ':') {
                pos++;
                placeholder::variadic_t ph;
                parse_variadic_options(ph);
                state = whatever;
                return ph;
            } else if (starts_with(pos, end, PH_END)) {
                pos += PH_END.size();
                state = whatever;
                return placeholder::variadic_t();
            } else if (is_variadic_legacy(*pos)) {
                std::cout << "Warning: using legacy '"
                          << *pos
                          << "' character in variadic placeholder is deprecated"
                          << std::endl;

                while (is_variadic_legacy(*pos)) {
                    pos++;
                }

                if (starts_with(pos, end, PH_END)) {
                    pos += PH_END.size();
                    ph.pattern = "'%k': %v";
                    state = whatever;
                    return ph;
                } else {
                    throw_<parser::illformed_t>();
                }
            }
        }

        throw_<parser::illformed_t>();
    }

    static
    bool
    is_variadic_legacy(char ch) {
        return ch == 'L' || ch == 'E' || ch == 'G' || ch == 'T' || ch == 'U';
    }

    void
    parse_variadic_options(placeholder::variadic_t& ph) {
        std::tie(ph.prefix, std::ignore) = parse({ ":" });
        std::string breaker;
        std::tie(ph.suffix, breaker) = parse({ ":", ")s" });
        if (breaker == ":") {
            std::tie(ph.separator, std::ignore) = parse({ ")s" });
        }
    }

    std::tuple<std::string, std::string>
    parse(std::initializer_list<std::string> breakers,
          std::function<int(int)> match = ::isprint) {
        std::string result;
        bool escaped = false;
        while (pos != end) {
            const char ch = *pos;
            if (!match(ch)) {
                throw_<parser::illformed_t>();
            }

            if (ch == '\\') {
                pos++;
                escaped = true;
                continue;
            }

            bool matched = false;
            std::string breaker;
            for (auto it = breakers.begin(); it != breakers.end(); ++it) {
                if (starts_with(pos, end, *it)) {
                    matched = true;
                    breaker = *it;
                    break;
                }
            }

            if (matched && !escaped) {
                pos += breaker.size();
                return std::make_tuple(result, breaker);
            }

            result.push_back(ch);
            escaped = false;
            pos++;
        }

        throw_<parser::illformed_t>();
    }

private:
    template<class Exception, class... Args>
    __attribute__((noreturn))
    void throw_(Args&&... args) {
        state = broken;
        throw Exception(
            std::distance(begin, pos),
            std::string(begin, end),
            std::forward<Args>(args)...
        );
    }

    template<typename Iterator, class Range>
    static
    inline
    bool
    starts_with(Iterator first, Iterator last, const Range& range) {
        return boost::starts_with(
            boost::make_iterator_range(first, last),
            range
        );
    }
};

} // namespace string

} // namespace formatter

} // namespace blackhole
