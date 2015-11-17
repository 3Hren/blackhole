#include <gtest/gtest.h>

#include <blackhole/formatter/string/parser.hpp>

using namespace blackhole::formatter::string;

TEST(parser_t, Literal) {
    parser_t parser("literal");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("literal", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, RequiredPlaceholder) {
    parser_t parser("%(id)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<placeholder::required_t>(*token).name);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, ThrowsExceptionIfRequiredPlaceholderIsNotFullyClosed) {
    parser_t parser("%(id");
    EXPECT_THROW(parser.next(), parser::illformed_t);
    EXPECT_THROW(parser.next(), parser::broken_t);
}

TEST(parser_t, ThrowsExceptionIfRequiredPlaceholderIsNotPartiallyClosed) {
    parser_t parser("%(id)");
    EXPECT_THROW(parser.next(), parser::illformed_t);
    EXPECT_THROW(parser.next(), parser::broken_t);
}

TEST(parser_t, ThrowsExceptionIfRequiredPlaceholderHasInvalidSymbols) {
    const std::vector<std::string> invalid = {
        "%(id )s",
        "%(id-)s",
        "%(id+)s",
        "%(id=)s"
    };

    for (auto it = invalid.begin(); it != invalid.end(); ++it) {
        parser_t parser(*it);

        EXPECT_THROW(parser.next(), parser::invalid_placeholder_t);
        EXPECT_THROW(parser.next(), parser::broken_t);
    }
}

TEST(parser_t, LiteralFollowedByRequiredPlaceholder) {
    parser_t parser("id=%(id)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id=", boost::get<literal_t>(*token).value);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<placeholder::required_t>(*token).name);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, RequiredPlaceholderFollowedByLiteral) {
    parser_t parser("%(id)s == id");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<placeholder::required_t>(*token).name);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ(" == id", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, RequiredPlaceholderSurroundedByLiterals) {
    parser_t parser("id=%(id)s definitely an id");

    auto token = parser.next();

    ASSERT_TRUE(!!token);
    EXPECT_EQ("id=", boost::get<literal_t>(*token).value);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<placeholder::required_t>(*token).name);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ(" definitely an id", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, OptionalPlaceholder) {
    parser_t parser("%(id::)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::optional_t>(*token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);

    EXPECT_FALSE(parser.next());
}

namespace testing {

struct optional_helper_t {
    std::string pattern;
    placeholder::optional_t expected;
};

}

TEST(parser_t, OptionalPlaceholderWithPrefix) {
    const std::vector<testing::optional_helper_t> fixtures = {
        { "%(id:/:)s", { "id", "/", "" } },
        { "%(id://:)s", { "id", "//", "" } },
    };

    for (auto it = fixtures.begin(); it != fixtures.end(); ++it) {
        parser_t parser(it->pattern);

        auto token = parser.next();
        ASSERT_TRUE(!!token);

        auto placeholder = boost::get<placeholder::optional_t>(*token);
        EXPECT_EQ(it->expected.name, placeholder.name);
        EXPECT_EQ(it->expected.prefix, placeholder.prefix);
        EXPECT_EQ(it->expected.suffix, placeholder.suffix);

        EXPECT_FALSE(parser.next());
    }
}

TEST(parser_t, OptionalPlaceholderWithSuffix) {
    parser_t parser("%(id::/)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::optional_t>(*token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, ThrowsWhenOptionalPlaceholderIsIllformed) {
    EXPECT_THROW(parser_t("%(id::").next(), parser::illformed_t);
}

TEST(parser_t, OptionalPlaceholderWithPrefixAndSuffix) {
    parser_t parser("%(id:/:/)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::optional_t>(*token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, OptionalPlaceholderWithPrefixAndSuffixEscaped) {
    parser_t parser("%(id:\\::\\:)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::optional_t>(*token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ(":", placeholder.prefix);
    EXPECT_EQ(":", placeholder.suffix);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, OptionalPlaceholderAny) {
    parser_t parser("%(id:id=:)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);


    auto placeholder = boost::get<placeholder::optional_t>(*token);
    EXPECT_EQ("id", placeholder.name);
    EXPECT_EQ("id=", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholder) {
    parser_t parser("%(...)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::value_t>(placeholder.pattern.at(2)));
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderFollowedByLiteral) {
    parser_t parser("%(...)s is great!");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ(" is great!", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, ThrowsExceptionOnIllformedVariadicPlaceholder) {
    EXPECT_THROW(parser_t("%(...").next(), parser::illformed_t);
    EXPECT_THROW(parser_t("%(....").next(), parser::illformed_t);
}

TEST(parser_t, VariadicPlaceholderWithPrefix) {
    parser_t parser("%(...:/:)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithSuffix) {
    parser_t parser("%(...::/)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithPrefixSuffix) {
    parser_t parser("%(...:/:/)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithSeparator) {
    parser_t parser("%(...:::.)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(".", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithPrefixSuffixSeparator) {
    parser_t parser("%(...:/:/: )s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(" ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithPrefixSuffixBraceAndSeparator) {
    parser_t parser("%(...:(:):, )s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("(", placeholder.prefix);
    EXPECT_EQ(")", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ(": ", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithPattern) {
    parser_t parser("%(...[%k=%v])s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ("=", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, ThrowsExceptionWhenVariadicPlaceholderIllformedAfterPattern) {
    parser_t parser("%(...[%k=%v].)s");

    EXPECT_THROW(parser.next(), parser::illformed_t);
    EXPECT_THROW(parser.next(), parser::broken_t);
}

TEST(parser_t, VariadicPlaceholderWithPatternPrefixSuffix) {
    parser_t parser("%(...[%k=%v]:/:/)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ("=", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderWithPatternPrefixSuffixSeparator) {
    parser_t parser("%(...[%k=%v]:/:/: )s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("/", placeholder.prefix);
    EXPECT_EQ("/", placeholder.suffix);
    ASSERT_EQ(3ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(0)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(1)));
    EXPECT_EQ("=", boost::get<literal_t>(placeholder.pattern.at(1)).value);
    EXPECT_EQ(" ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, VariadicPlaceholderLegacy) {
    parser_t parser("%(...L)s");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    auto placeholder = boost::get<placeholder::variadic_t>(*token);
    EXPECT_EQ("", placeholder.prefix);
    EXPECT_EQ("", placeholder.suffix);
    ASSERT_EQ(4ULL, placeholder.pattern.size());
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(0)));
    EXPECT_EQ("'", boost::get<literal_t>(placeholder.pattern.at(0)).value);
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::key_t>(placeholder.pattern.at(1)));
    EXPECT_NO_THROW(boost::get<literal_t>(placeholder.pattern.at(2)));
    EXPECT_EQ("': ", boost::get<literal_t>(placeholder.pattern.at(2)).value);
    EXPECT_NO_THROW(boost::get<placeholder::variadic_t::value_t>(placeholder.pattern.at(3)));
    EXPECT_EQ(", ", placeholder.separator);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, ThrowsExceptionWhenVariadicPlaceholderLegacyIllformed) {
    parser_t parser("%(...L");

    EXPECT_THROW(parser.next(), parser::illformed_t);
    EXPECT_THROW(parser.next(), parser::broken_t);
}
