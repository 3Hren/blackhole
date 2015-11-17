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
        return token_t(parse_literal());
    case state_t::placeholder:
        return parse_placeholder();
    case state_t::broken:
        throw_<broken_t>();
    }
}

auto
parser_t::parse_unknown() -> boost::optional<token_t> {
    if (exact("{")) {
        if (exact(std::next(pos), "{")) {
            // The "{{" will be treated as a single "{", so don't advance the cursor.
            state = state_t::literal;
        } else {
            ++pos;
            state = state_t::placeholder;
        }
    } else {
        state = state_t::literal;
    }

    return next();
}

auto
parser_t::parse_literal() -> literal_t {
    std::string result;

    while (pos != std::end(pattern)) {
        // The "{{" and "}}" are treated as a single "{" and "}" respectively.
        if (exact("{{") || exact("}}")) {
            result.push_back(*pos);
            ++pos;
            ++pos;
            continue;
        } else if (exact("{")) {
            state = state_t::placeholder;
            ++pos;
            return {result};
        } else if (exact("}")) {
            // Unclosed single "}".
            throw_<illformed_t>();
        }

        result.push_back(*pos);
        ++pos;
    }

    return {result};
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
                    return parse_spec(ph::message_t{std::move(spec)});
                } else if (name == "severity") {
                    auto token = parse_spec(ph::severity<user>{std::move(spec)});
                    auto transformed = boost::get<ph::severity<user>>(token);
                    if (boost::ends_with(transformed.spec, "d}")) {
                        return ph::severity<num>{transformed.spec};
                    } else {
                        return token;
                    }
                } else if (name == "timestamp") {
                    auto token = parse_spec(ph::timestamp<user>{{}, std::move(spec)});
                    auto transformed = boost::get<ph::timestamp<user>>(token);
                    if (boost::ends_with(transformed.spec, "d}")) {
                        return ph::timestamp<num>{transformed.pattern, transformed.spec};
                    } else {
                        return token;
                    }
                } else {
                    return parse_spec(ph::generic_t{std::move(name), std::move(spec)});
                }
            } else if (exact("}")) {
                pos += 1;
                state = state_t::unknown;

                spec.push_back('}');
                if (boost::starts_with(name, "...")) {
                    return ph::leftover_t{std::move(name)};
                } else if (name == "message") {
                    return ph::message_t{std::move(spec)};
                } else if (name == "severity") {
                    return ph::severity<user>{std::move(spec)};
                } else if (name == "timestamp") {
                    return ph::timestamp<user>{{}, std::move(spec)};
                }

                return ph::generic_t{std::move(name), std::move(spec)};
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
        if (exact("}")) {
            pos += 1;
            state = state_t::unknown;
            return token;
        }

        ++pos;
    }

    return token;
}

template<typename T>
auto
parser_t::parse_spec(ph::timestamp<T> token) -> token_t {
    if (exact("{")) {
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

    return parse_spec<ph::timestamp<T>>(std::move(token));
}

template<typename Range>
auto
parser_t::exact(const Range& range) const -> bool {
    return exact(pos, range);
}

template<typename Range>
auto
parser_t::exact(const_iterator pos, const Range& range) const -> bool {
    return starts_with(pos, std::end(pattern), range);
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
