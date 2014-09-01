#include <array>
#include <ctime>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "global.hpp"

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

static const std::array<char, 2> PLACEHOLDER_BEGIN = {{ '%', '(' }};
static const std::array<char, 2> PLACEHOLDER_END   = {{ ')', 's' }};
static const std::array<char, 3> PLACEHOLDER_VARIADIC = {{ '.', '.', '.' }};

class parser_t {
    enum state_t {
        unknown,
        literal,
        placeholder,
        broken
    };

    std::string pattern;
    std::string::const_iterator pos;
    std::string::const_iterator end;
    state_t state;

public:
    parser_t(std::string pattern) :
        pattern(std::move(pattern)),
        pos(this->pattern.begin()),
        end(this->pattern.end()),
        state(unknown)
    {}

    token_t
    next() {
        if (pos == end) {
            throw std::runtime_error("EOF");
        }

        switch (state) {
        case unknown:
            return parse_unknown();
        case literal:
            return parse_literal();
        case placeholder:
            return parse_placeholder();
        case broken:
            throw std::runtime_error("broken parser");
        default:
            BOOST_ASSERT(false);
        }
    }

    token_t
    parse_unknown() {
        if (boost::starts_with(boost::make_iterator_range(pos, end), PLACEHOLDER_BEGIN)) {
            pos += PLACEHOLDER_BEGIN.size();
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
            if (boost::starts_with(boost::make_iterator_range(pos, end), PLACEHOLDER_BEGIN)) {
                pos += PLACEHOLDER_BEGIN.size();
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
        if (boost::starts_with(boost::make_iterator_range(pos, end), PLACEHOLDER_VARIADIC)) {
            pos += PLACEHOLDER_VARIADIC.size();
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

                    placeholder::optional_t placeholder;
                    placeholder.name = name;
                    std::tie(placeholder.prefix, std::ignore) = parse({ ":" });
                    std::tie(placeholder.suffix, std::ignore) = parse({ ")s" });
                    state = unknown;
                    return placeholder;
                } else if (boost::starts_with(boost::make_iterator_range(pos, end), PLACEHOLDER_END)) {
                    pos += PLACEHOLDER_END.size();
                    state = unknown;
                    return token_t(placeholder::required_t { name });
                } else {
                    state = broken;
                    throw std::runtime_error("invalid placeholder name");
                }
            }

            pos++;
        }

        return token_t(placeholder::required_t { name });
    }

    placeholder::variadic_t
    parse_variadic() {
        if (pos == end) {
            state = broken;
            throw std::runtime_error("invalid format");
        } else if (*pos == '[') {
            pos++;

            placeholder::variadic_t ph;
            std::tie(ph.pattern, std::ignore) = parse({ "]" });
            if (boost::starts_with(boost::make_iterator_range(pos, end), PLACEHOLDER_END)) {
                pos += PLACEHOLDER_END.size();
                state = unknown;
                return ph;
            } else if (*pos == ':') {
                pos++;
                std::tie(ph.prefix, std::ignore) = parse({ ":" });
                std::string breaker;
                std::tie(ph.suffix, breaker) = parse({ ":", ")s" });
                if (breaker == ":") {
                    std::tie(ph.separator, std::ignore) = parse({ ")s" });
                }
                state = unknown;
                return ph;
            } else {
                state = broken;
                throw std::runtime_error("invalid format");
            }
        } else if (*pos == ':') {
            pos++;

            placeholder::variadic_t ph;
            std::tie(ph.prefix, std::ignore) = parse({ ":" });
            std::string breaker;
            std::tie(ph.suffix, breaker) = parse({ ":", ")s" });
            if (breaker == ":") {
                std::tie(ph.separator, std::ignore) = parse({ ")s" });
            }

            state = unknown;
            return ph;
        } else if (boost::starts_with(boost::make_iterator_range(pos, end), PLACEHOLDER_END)) {
            pos += PLACEHOLDER_END.size();
            return placeholder::variadic_t();
        }

        state = broken;
        throw std::runtime_error("invalid format");
    }

    std::tuple<std::string, std::string>
    parse(std::initializer_list<std::string> breakers) {
        std::string result;
        bool escaped = false;
        while (pos != end) {
            const char ch = *pos;
            if (ch == '\\') {
                pos++;
                escaped = true;
                continue;
            }

            bool matched = false;
            std::string breaker;
            for (auto it = breakers.begin(); it != breakers.end(); ++it) {
                if (boost::starts_with(boost::make_iterator_range(pos, end),
                                       boost::make_iterator_range(it->begin(), it->end())))
                {
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

        state = broken;
        throw std::runtime_error("invalid format");
    }
};

TEST(parser_t, Literal) {
    parser_t parser("literal");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<literal_t>(token));
    EXPECT_EQ("literal", boost::get<literal_t>(token).value);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, RequiredPlaceholder) {
    parser_t parser("%(id)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::required_t>(token));
    EXPECT_EQ("id", boost::get<placeholder::required_t>(token).name);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, ThrowsExceptionIfRequiredPlaceholderHasInvalidSymbols) {
    const std::vector<std::string> invalid = {
        "%(id )s",
        "%(id-)s",
        "%(id_)s",
        "%(id+)s",
        "%(id=)s"
    };

    for (auto it = invalid.begin(); it != invalid.end(); ++it) {
        parser_t parser(*it);

        EXPECT_THROW(parser.next(), std::runtime_error);
        EXPECT_THROW(parser.next(), std::runtime_error);
    }
}

TEST(parser_t, LiteralFollowedByRequiredPlaceholder) {
    parser_t parser("id=%(id)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<literal_t>(token));
    EXPECT_EQ("id=", boost::get<literal_t>(token).value);

    token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::required_t>(token));
    EXPECT_EQ("id", boost::get<placeholder::required_t>(token).name);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, RequiredPlaceholderFollowedByLiteral) {
    parser_t parser("%(id)s == id");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::required_t>(token));
    EXPECT_EQ("id", boost::get<placeholder::required_t>(token).name);

    token = parser.next();

    ASSERT_NO_THROW(boost::get<literal_t>(token));
    EXPECT_EQ(" == id", boost::get<literal_t>(token).value);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, RequiredPlaceholderSurroundedByLiterals) {
    parser_t parser("id=%(id)s definitely an id");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<literal_t>(token));
    EXPECT_EQ("id=", boost::get<literal_t>(token).value);

    token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::required_t>(token));
    EXPECT_EQ("id", boost::get<placeholder::required_t>(token).name);

    token = parser.next();

    ASSERT_NO_THROW(boost::get<literal_t>(token));
    EXPECT_EQ(" definitely an id", boost::get<literal_t>(token).value);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, OptionalPlaceholder) {
    parser_t parser("%(id::)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::optional_t>(token));
    auto placeholder = boost::get<placeholder::optional_t>(token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, OptionalPlaceholderWithPrefix) {
    static const struct {
        std::string pattern;
        placeholder::optional_t expected;
    } FIXTURES[] = {
        { "%(id:/:)s", { "id", "/", "" } },
        { "%(id://:)s", { "id", "//", "" } },
    };

    for (auto fixture : FIXTURES) {
        parser_t parser(fixture.pattern);

        auto token = parser.next();

        ASSERT_NO_THROW(boost::get<placeholder::optional_t>(token));
        auto placeholder = boost::get<placeholder::optional_t>(token);
        EXPECT_EQ(fixture.expected.name, placeholder.name);
        EXPECT_EQ(fixture.expected.prefix, placeholder.prefix);
        EXPECT_EQ(fixture.expected.suffix, placeholder.suffix);

        EXPECT_THROW(parser.next(), std::runtime_error);
    }
}

TEST(parser_t, OptionalPlaceholderWithSuffix) {
    parser_t parser("%(id::/)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::optional_t>(token));
    auto placeholder = boost::get<placeholder::optional_t>(token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, OptionalPlaceholderWithPrefixAndSuffix) {
    parser_t parser("%(id:/:/)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::optional_t>(token));
    auto placeholder = boost::get<placeholder::optional_t>(token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, OptionalPlaceholderWithPrefixAndSuffixEscaped) {
    parser_t parser("%(id:\\::\\:)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::optional_t>(token));
    auto placeholder = boost::get<placeholder::optional_t>(token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ(":", placeholder.prefix);
    EXPECT_EQ(":", placeholder.suffix);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, OptionalPlaceholderAny) {
    parser_t parser("%(id:id=:)s");

    auto token = parser.next();

    ASSERT_NO_THROW(boost::get<placeholder::optional_t>(token));
    auto placeholder = boost::get<placeholder::optional_t>(token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("id=", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholder) {
    parser_t parser("%(...)s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, InvalidVariadicPlaceholder) {
    parser_t parser("%(...");

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPrefix) {
    parser_t parser("%(...:/:)s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithSuffix) {
    parser_t parser("%(...::/)s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPrefixSuffix) {
    parser_t parser("%(...:/:/)s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithSeparator) {
    parser_t parser("%(...:::.)s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(".", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPrefixSuffixSeparator) {
    parser_t parser("%(...:/:/: )s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(" ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPrefixSuffixBraceAndSeparator) {
    parser_t parser("%(...:(:):, )s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("(", placeholder.prefix);
    EXPECT_EQ(")", placeholder.suffix);
    EXPECT_EQ("%k: %v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPattern) {
    parser_t parser("%(...[%k=%v])s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    EXPECT_EQ("%k=%v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPatternPrefixSuffix) {
    parser_t parser("%(...[%k=%v]:/:/)s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    EXPECT_EQ("%k=%v", placeholder.pattern);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}

TEST(parser_t, VariadicPlaceholderWithPatternPrefixSuffixSeparator) {
    parser_t parser("%(...[%k=%v]:/:/: )s");

    auto token = parser.next();
    auto placeholder = boost::get<placeholder::variadic_t>(token);

    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    EXPECT_EQ("%k=%v", placeholder.pattern);
    EXPECT_EQ(" ", placeholder.separator);

    EXPECT_THROW(parser.next(), std::runtime_error);
}
