#include "blackhole/detail/formatter/string/parser.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/variant/get.hpp>

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
    state(state_t::unknown),
    pattern(std::move(pattern)),
    pos(std::begin(this->pattern))
{}

auto
parser_t::next() -> boost::optional<token_t> {
    if (state == state_t::broken) {
        throw_<broken_t>();
    }

    if (pos == std::end(pattern)) {
        return boost::none;
    }

    switch (state) {
    case state_t::unknown:
        return parse_unknown();
    case state_t::literal:
        return parse_literal();
    case state_t::placeholder:
        return parse_placeholder();
    case state_t::broken:
        throw_<broken_t>();
    }

    throw_<broken_t>();
}

auto
parser_t::parse_unknown() -> boost::optional<token_t> {
    if (starts_with(pos, std::end(pattern), "{") && !starts_with(pos + 1, std::end(pattern), "{")) {
        ++pos;
        state = state_t::placeholder;
    } else {
        state = state_t::literal;
    }

    return next();
}

auto
parser_t::parse_literal() -> token_t {
    literal_t literal;

    while (pos != std::end(pattern)) {
        if (starts_with(pos, std::end(pattern), "{{") || starts_with(pos, std::end(pattern), "}}")) {
            pos += 1;
        } else if (starts_with(pos, std::end(pattern), "{")) {
            pos += 1;
            state = state_t::placeholder;
            return literal;
        } else if (starts_with(pos, std::end(pattern), "}")) {
            throw_<illformed_t>();
        }

        literal.value.push_back(*pos);
        ++pos;
    }

    return literal;
}

auto
parser_t::parse_placeholder() -> token_t {
    std::string name;
    std::string spec("{");

    while (pos != std::end(pattern)) {
        const auto ch = *pos;

        if (std::isalpha(ch) || std::isdigit(ch) || ch == '_' || ch == '.') {
            name.push_back(ch);
        } else {
            if (ch == ':') {
                spec.push_back(ch);
                ++pos;

                // TODO: Consider token factory.
                if (name == "message") {
                    return parse_spec(placeholder::message_t{std::move(spec)});
                } else if (name == "severity") {
                    auto token = parse_spec(placeholder::severity_t{std::move(spec)});
                    auto transformed = boost::get<placeholder::severity_t>(token);
                    if (boost::ends_with(transformed.spec, "d}")) {
                        return placeholder::numeric_severity_t{transformed.spec};
                    } else {
                        return token;
                    }
                } else if (name == "timestamp") {
                    return parse_spec(placeholder::timestamp_t{{}, std::move(spec)});
                } else {
                    return parse_spec(placeholder::common_t{std::move(name), std::move(spec)});
                }
            } else if (starts_with(pos, std::end(pattern), "}")) {
                pos += 1;
                state = state_t::unknown;

                spec.push_back('}');
                if (boost::starts_with(name, "...")) {
                    return placeholder::leftover_t{std::move(name)};
                } else if (name == "message") {
                    return placeholder::message_t{std::move(spec)};
                } else if (name == "severity") {
                    return placeholder::severity_t{std::move(spec)};
                } else if (name == "timestamp") {
                    return placeholder::timestamp_t{{}, std::move(spec)};
                }

                return placeholder::common_t{std::move(name), std::move(spec)};
            } else {
                throw_<invalid_placeholder_t>();
            }
        }

        pos++;
    }

    throw_<illformed_t>();
}

template<typename T>
auto
parser_t::parse_spec(T token) -> token_t {
    while (pos != std::end(pattern)) {
        const auto ch = *pos;

        token.spec.push_back(ch);

        // TODO: Here it's the right place to validate spec format, but now I don't have much
        // time to implement it.
        if (starts_with(pos, std::end(pattern), "}")) {
            pos += 1;
            state = state_t::unknown;
            return token;
        }

        ++pos;
    }

    return token;
}

auto
parser_t::parse_spec(placeholder::timestamp_t token) -> token_t {
    if (starts_with(pos, std::end(pattern), "{")) {
        ++pos;

        while (pos != std::end(pattern)) {
            const auto ch = *pos;

            if (ch == '}') {
                ++pos;
                break;
            }

            token.pattern.push_back(*pos);
            ++pos;
        }
    }

    return parse_spec<placeholder::timestamp_t>(std::move(token));
}

template<class Exception, class... Args>
__attribute__((noreturn))
auto
parser_t::throw_(Args&&... args) -> void {
    state = state_t::broken;
    throw Exception(static_cast<std::size_t>(std::distance(std::begin(pattern), pos)), pattern,
        std::forward<Args>(args)...);
}

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
