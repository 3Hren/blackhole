#include <array>
#include <ctime>
#include <stdexcept>

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

}

typedef boost::variant<
    literal_t,
    placeholder::required_t,
    placeholder::optional_t
> token_t;

static const std::array<char, 2> PLACEHOLDER_BEGIN = {{ '%', '(' }};
static const std::array<char, 2> PLACEHOLDER_END   = {{ ')', 's' }};

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

    token_t next() {
        if (pos == end) {
            throw std::runtime_error("end");
        }

        switch (state) {
        case unknown:
            if (equal(pos, end, PLACEHOLDER_BEGIN.begin(), PLACEHOLDER_BEGIN.end())) {
                pos += PLACEHOLDER_BEGIN.size();
                state = placeholder;
            } else {
                state = literal;
            }
            return next();
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

    literal_t parse_literal() {
        literal_t literal;

        while (pos != end) {
            if (equal(pos, end, PLACEHOLDER_BEGIN.begin(), PLACEHOLDER_BEGIN.end())) {
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
        // test for variadic, otherwise either required or optional.

        std::string name;
        while (pos != end) {
            const char ch = *pos;
            std::cout << ":=" << ch << std::endl;
            if (std::isalpha(ch) || std::isdigit(ch)) {
                name.push_back(ch);
            } else {
                if (ch == ':') {
                    placeholder::optional_t placeholder;
                    placeholder.name = name;

                    pos++;
                    placeholder.prefix = parse_prefix();
                    placeholder.suffix = parse_suffix();

                    // maybe format
                    if (!equal(pos, end, PLACEHOLDER_END.begin(), PLACEHOLDER_END.end())) {
                        throw std::runtime_error("!");
                    }
                    pos += PLACEHOLDER_END.size();
                    state = unknown;
                    return placeholder;
                } else if (equal(pos, end, PLACEHOLDER_END.begin(), PLACEHOLDER_END.end())) {
                    pos += PLACEHOLDER_END.size();
                    state = unknown;
                    return token_t(placeholder::required_t { name });
                } else {
                    // throw
                    state = broken;
                    throw std::runtime_error("invalid placeholder name");
                }
            }

            pos++;
        }

        return token_t(placeholder::required_t { name });
    }

    std::string parse_prefix() {
        std::string prefix;
        bool escaped = false;
        while (pos != end) {
            const char ch = *pos;
            if (ch == '\\') {
                escaped = true;
                pos++;
                continue;
            }

            if (ch == ':' && !escaped) {
                pos++;
                return prefix;
            }

            prefix.push_back(ch);
            escaped = false;
            pos++;
        }

        return prefix;
    }

    std::string parse_suffix() {
        std::string suffix;
        bool escaped = false;
        while (pos != end) {
            const char ch = *pos;
            if (ch == '\\') {
                escaped = true;
                pos++;
                continue;
            }

            if (ch == ')') {
                return suffix;
            }

            suffix.push_back(ch);
            escaped = false;
            pos++;
        }

        return suffix;
    }

    template<class InputIterator, class OtherIterator>
    static
    bool
    equal(InputIterator first1, InputIterator last1, OtherIterator first2, OtherIterator last2) {
        for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
            if (*first1 != *first2) {
                return false;
            }
        }

        return true;
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

// %(id)s -> "42" | throw
// id=%(id)s -> "id=42" | throw

// %(id::)s -> "42" | ""
// %(id:/:/)s -> "/42/" | ""
// %(id:\::\:)s -> ":42:" | ""
// %(id:id=:)s -> "id=42" | ""

// %(...)s -> "id: 42" | ""
// %(...:(:))s -> "(id: 42, message: le message)" | ""
// %(...[%k=%v]:(:):,)s -> "(id=42,message=le message)" |""
