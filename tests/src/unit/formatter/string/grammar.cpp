#include <gtest/gtest.h>

#include <boost/spirit/home/qi/operator/expect.hpp>

#include <src/formatter/string/error.hpp>
#include <src/formatter/string/grammar.hpp>
#include <src/formatter/string/grammar.inl.hpp>

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace string {
namespace {

TEST(grammar_t, NoExceptOnEmptyPattern) {
    const auto r = parse("{}");

    EXPECT_FALSE(!!r.pattern());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("", r.spec);
}

TEST(grammar_t, ExtractSpec) {
    const auto r = parse("{:<20s}");

    EXPECT_FALSE(!!r.pattern());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("<20s", r.spec);
}

TEST(grammar_t, ThrowsIfPatternNotClosed) {
    EXPECT_THROW(parse("{"), parser_error_t);
}

TEST(grammar_t, ThrowsOnMalformedBeginSymbol) {
    EXPECT_THROW(parse("0{}"), parser_error_t);
}

TEST(grammar_t, ThrowsOnExcessSymbolAfterCloseBrace) {
    EXPECT_THROW(parse("{}0"), parser_error_t);
}

TEST(grammar_t, ExtractPattern) {
    const auto r = parse("{:{{name}={value}:p}s}");

    EXPECT_EQ("{name}={value}", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractEmptyPatternWithGlobalTypeSpec) {
    const auto r = parse("{:{:p}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ("", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractPatternDefault) {
    const auto r = parse("{:{{name}: {value}:p}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ("{name}: {value}", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ThrowsOnUnknownPatternTypeAsUnregistered) {
    EXPECT_THROW(parse("{:{{name}: {value}:l}s}"), parser_error_t);
}

TEST(grammar_t, ThrowsOnUnspecifiedPatternType) {
    EXPECT_THROW(parse("{:{{name}: {value}:}}"), parser_error_t);
}

TEST(grammar_t, ExtractPatternWithCurlyBraces) {
    const auto r = parse("{:{{name}: {value} {{{name}}:<20}:p}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ("{name}: {value} {{{name}}:<20}", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractPatternsWithCurlyBracesWithSpecType) {
    const auto r = parse("{:{{{}}:p}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ("{{}}", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ThrowsOnExtractPatternsWithCurlyBracesWithoutSpecType) {
    EXPECT_THROW(parse("{:{{{}}:p}}"), parser_error_t);
}

TEST(grammar_t, ExtractPattenWithTypeMock) {
    const auto r = parse("{:{:p:s:p:p}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ(":p:s:p", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractPatternWithTypeMockWithCurlyBraces) {
    const auto r = parse("{:{:p}}:p}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ(":p}}", r.pattern().get());
    EXPECT_FALSE(!!r.separator());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractSeparatorOnly) {
    const auto r = parse("{:{\t:s}s}");

    EXPECT_FALSE(!!r.pattern());
    ASSERT_TRUE(!!r.separator());
    EXPECT_EQ("\t", r.separator().get());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractEmptyPatternWithSeparator) {
    const auto r = parse("{:{:p}{\t:s}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ("", r.pattern().get());
    ASSERT_TRUE(!!r.separator());
    EXPECT_EQ("\t", r.separator().get());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractPatternWithSeparator) {
    const auto r = parse("{:{{name}={value}:p}{\t:s}s}");

    ASSERT_TRUE(!!r.pattern());
    EXPECT_EQ("{name}={value}", r.pattern().get());
    ASSERT_TRUE(!!r.separator());
    EXPECT_EQ("\t", r.separator().get());
    EXPECT_EQ("s", r.spec);
}

TEST(grammar_t, ExtractSeparatorEmpty) {
    const auto r = parse("{:{:s}s}");

    EXPECT_FALSE(!!r.pattern());
    ASSERT_TRUE(!!r.separator());
    EXPECT_EQ("", r.separator().get());
    EXPECT_EQ("s", r.spec);
}

TEST(pattern_grammar_t, KeyValue) {
    const auto r = parse_pattern("{name}={value}");

    ASSERT_EQ(3, r.size());
    EXPECT_EQ("", boost::get<ph::attribute<name>>(r.at(0)).spec);
    EXPECT_EQ("=", boost::get<literal_t>(r.at(1)).value);
    EXPECT_EQ("", boost::get<ph::attribute<value>>(r.at(2)).spec);
}

TEST(pattern_grammar_t, WithBraces) {
    const auto r = parse_pattern("{{{name}: {value}}}");

    ASSERT_EQ(5, r.size());
    EXPECT_EQ("{", boost::get<literal_t>(r.at(0)).value);
    EXPECT_EQ("", boost::get<ph::attribute<name>>(r.at(1)).spec);
    EXPECT_EQ(": ", boost::get<literal_t>(r.at(2)).value);
    EXPECT_EQ("", boost::get<ph::attribute<value>>(r.at(3)).spec);
    EXPECT_EQ("}", boost::get<literal_t>(r.at(4)).value);
}

TEST(pattern_grammar_t, NameWithSpec) {
    const auto r = parse_pattern("{name:<20s}");

    ASSERT_EQ(1, r.size());
    EXPECT_EQ("<20s", boost::get<ph::attribute<name>>(r.at(0)).spec);
}

TEST(pattern_grammar_t, ValueWithSpec) {
    const auto r = parse_pattern("{value:<20s}");

    ASSERT_EQ(1, r.size());
    EXPECT_EQ("<20s", boost::get<ph::attribute<value>>(r.at(0)).spec);
}

TEST(pattern_grammar_t, ThrowsOnUnknownPlaceholder) {
    EXPECT_THROW(parse_pattern("{unknown}"), std::runtime_error);
}

// TODO: Check every fucking error.

TEST(parse_leftover, Conversion) {
    const auto ph = parse_leftover("{:{{name:5s}={value:^10s}:p}{\t:s}<50s}");

    EXPECT_EQ("{:<50s}", ph.spec);
    ASSERT_EQ(3, ph.tokens.size());
    EXPECT_EQ("5s", boost::get<ph::attribute<name>>(ph.tokens.at(0)).spec);
    EXPECT_EQ("{:5s}", boost::get<ph::attribute<name>>(ph.tokens.at(0)).format);
    EXPECT_EQ("=", boost::get<literal_t>(ph.tokens.at(1)).value);
    EXPECT_EQ("^10s", boost::get<ph::attribute<value>>(ph.tokens.at(2)).spec);
    EXPECT_EQ("{:^10s}", boost::get<ph::attribute<value>>(ph.tokens.at(2)).format);
    EXPECT_EQ("\t", ph.separator);
}

}  // namespace
}  // namespace string
}  // namespace formatter
}  // namespace v1
}  // namespace blackhole
