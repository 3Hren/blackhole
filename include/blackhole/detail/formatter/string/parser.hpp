#pragma once

#include <boost/optional/optional.hpp>
#include <boost/variant/variant.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/detail/formatter/string/error.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

struct literal_t {
    std::string value;
};

struct placeholder_t {
    std::string name;
    std::string spec;
};

typedef boost::variant<
    literal_t,
    placeholder_t
> token_t;

class parser_t {
    enum state_t {
        /// Undetermined state.
        whatever,
        literal,
        placeholder,
        broken
    };

    const std::string pattern;

    std::string::const_iterator pos;
    state_t state;

public:
    explicit parser_t(std::string pattern) :
        pattern(std::move(pattern)),
        pos(this->pattern.begin()),
        state(whatever)
    {}

    auto begin() const -> std::string::const_iterator {
        return pattern.begin();
    }

    auto end() const noexcept -> std::string::const_iterator {
        return pattern.end();
    }

    auto next() -> boost::optional<token_t> {
        if (state == broken) {
            throw_<broken_parser>();
        }

        if (pos == end()) {
            return boost::none;
        }

       switch (state) {
        case whatever:
            return parse_whatever();
        case literal:
            return parse_literal();
        case placeholder:
            return parse_placeholder();
        case broken:
            throw_<broken_parser>();
        default:
            BOOST_ASSERT(false);
        }

        throw_<broken_parser>();
    }

private:
    auto parse_whatever() -> boost::optional<token_t> {
        if (starts_with(pos, end(), "{{")) {
            state = literal;
        } else if (starts_with(pos, end(), "{")) {
            pos += 1;
            state = placeholder;
        } else {
            state = literal;
        }

        return next();
    }

    auto parse_literal() -> token_t {
        literal_t literal;

        while (pos != end()) {
            if (starts_with(pos, end(), "{{") || starts_with(pos, end(), "}}")) {
                pos += 1;
            } else if (starts_with(pos, end(), "{")) {
                pos += 1;
                state = placeholder;
                return literal;
            } else if (starts_with(pos, end(), "}")) {
                throw_<illformed>();
            }

            literal.value.push_back(*pos);
            ++pos;
        }

        return literal;
    }

    auto parse_placeholder() -> token_t {
        placeholder_t result;

        while (pos != end()) {
            const auto ch = *pos;

            if (std::isalpha(ch) || std::isdigit(ch) || ch == '_') {
                result.name.push_back(ch);
            } else {
                if (ch == ':') {
                    result.spec.push_back(ch);
                    ++pos;
                    return parse_spec(std::move(result));
                } else if (starts_with(pos, end(), "}")) {
                    pos += 1;
                    state = whatever;
                    return result;
                } else {
                    throw_<invalid_placeholder>();
                }
            }

            pos++;
        }

        throw_<illformed>();
    }

    auto parse_spec(placeholder_t placeholder) -> token_t {
        while (pos != end()) {
            const auto ch = *pos;

            // TODO: Here it's the right place to validate spec format, but now I don't have much
            // time to implement it.
            if (starts_with(pos, end(), "}")) {
                pos += 1;
                state = whatever;
                return placeholder;
            }

            placeholder.spec.push_back(ch);
            ++pos;
        }

        return placeholder;
    }

    template<class Exception, class... Args>
    __attribute__((noreturn)) auto throw_(Args&&... args) -> void {
        state = broken;
        throw Exception(std::distance(begin(), pos), pattern, std::forward<Args>(args)...);
    }

    template<typename Iterator, class Range>
    static auto starts_with(Iterator first, Iterator last, const Range& range) -> bool {
        return boost::starts_with(boost::make_iterator_range(first, last), range);
    }
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
