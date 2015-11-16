#include "blackhole/detail/formatter/string/parser.hpp"

#include <boost/algorithm/string.hpp>

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

namespace {

template<typename Iterator, class Range>
static auto starts_with(Iterator first, Iterator last, const Range& range) -> bool {
    return boost::starts_with(boost::make_iterator_range(first, last), range);
}

}  // namespace

parser_t::parser_t(std::string pattern) :
    pattern(std::move(pattern)),
    state(state_t::whatever),
    pos(this->pattern.begin())
{}

auto
parser_t::next() -> boost::optional<token_t> {
    if (state == state_t::broken) {
        throw_<broken_parser>();
    }

    if (pos == end()) {
        return boost::none;
    }

    switch (state) {
    case state_t::whatever:
        return parse_unknown();
    case state_t::literal:
        return parse_literal();
    case state_t::placeholder:
        return parse_placeholder();
    case state_t::broken:
        throw_<broken_parser>();
    }

    throw_<broken_parser>();
}

auto
parser_t::begin() const -> std::string::const_iterator {
    return pattern.begin();
}

auto
parser_t::end() const -> std::string::const_iterator {
    return pattern.end();
}

auto
parser_t::parse_unknown() -> boost::optional<token_t> {
    if (starts_with(pos, end(), "{{")) {
        state = state_t::literal;
    } else if (starts_with(pos, end(), "{")) {
        pos += 1;
        state = state_t::placeholder;
    } else {
        state = state_t::literal;
    }

    return next();
}

auto
parser_t::parse_literal() -> token_t {
    literal_t literal;

    while (pos != end()) {
        if (starts_with(pos, end(), "{{") || starts_with(pos, end(), "}}")) {
            pos += 1;
        } else if (starts_with(pos, end(), "{")) {
            pos += 1;
            state = state_t::placeholder;
            return literal;
        } else if (starts_with(pos, end(), "}")) {
            throw_<illformed>();
        }

        literal.value.push_back(*pos);
        ++pos;
    }

    return literal;
}

auto
parser_t::parse_placeholder() -> token_t {
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
                state = state_t::whatever;
                return result;
            } else {
                throw_<invalid_placeholder>();
            }
        }

        pos++;
    }

    throw_<illformed>();
}

auto
parser_t::parse_spec(placeholder_t placeholder) -> token_t {
    while (pos != end()) {
        const auto ch = *pos;

        // TODO: Here it's the right place to validate spec format, but now I don't have much
        // time to implement it.
        if (starts_with(pos, end(), "}")) {
            pos += 1;
            state = state_t::whatever;
            return placeholder;
        }

        placeholder.spec.push_back(ch);
        ++pos;
    }

    return placeholder;
}

template<class Exception, class... Args>
__attribute__((noreturn))
auto
parser_t::throw_(Args&&... args) -> void {
    state = state_t::broken;
    throw Exception(static_cast<std::size_t>(std::distance(begin(), pos)), pattern,
        std::forward<Args>(args)...);
}

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
