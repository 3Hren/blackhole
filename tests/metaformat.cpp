#include <gtest/gtest.h>

#include "blackhole/extensions/format.hpp"
#include <blackhole/extensions/metaformat2.hpp>

namespace blackhole {
namespace testing {

using namespace blackhole::experimental;

using blackhole::experimental::detail::tokenizer;
using blackhole::experimental::detail::tokenizer_t;

TEST(Tokenizer, Empty) {
    constexpr auto actual = tokenizer_t("").count();

    EXPECT_EQ(0, actual.tokens());
    EXPECT_EQ(0, actual.literals());
    EXPECT_EQ(0, actual.placeholders());
}

TEST(Tokenizer, SingleLiteral) {
    constexpr auto actual = tokenizer_t("exposing service on local endpoint").count();

    EXPECT_EQ(1, actual.tokens());
    EXPECT_EQ(1, actual.literals());
    EXPECT_EQ(0, actual.placeholders());
}

TEST(TokenCount, SingleWithBraceSymbols) {
    constexpr auto actual = tokenizer_t("service {{ name: storage }}").count();

    EXPECT_EQ(2, actual.tokens());
    EXPECT_EQ(2, actual.literals());
    EXPECT_EQ(0, actual.placeholders());
}

TEST(Tokenizer, SingleWithBraceSymbolsNotInTheEnd) {
    constexpr auto actual = tokenizer_t("service {{ name: storage }} has been started").count();

    EXPECT_EQ(3, actual.tokens());
    EXPECT_EQ(3, actual.literals());
    EXPECT_EQ(0, actual.placeholders());
}

TEST(Tokenizer, SinglePlaceholder) {
    constexpr auto actual = tokenizer_t("{}").count();

    EXPECT_EQ(1, actual.tokens());
    EXPECT_EQ(0, actual.literals());
    EXPECT_EQ(1, actual.placeholders());
}

TEST(Tokenizer, TwoPlaceholders) {
    constexpr auto actual = tokenizer_t("{}{}").count();

    EXPECT_EQ(2, actual.tokens());
    EXPECT_EQ(0, actual.literals());
    EXPECT_EQ(2, actual.placeholders());
}

TEST(Tokenizer, SinglePlaceholderAtTheEnd) {
    constexpr auto actual = tokenizer_t("exposing service on local endpoint {}").count();

    EXPECT_EQ(2, actual.tokens());
    EXPECT_EQ(1, actual.literals());
    EXPECT_EQ(1, actual.placeholders());
}

TEST(Tokenizer, SinglePlaceholderAtTheBeginning) {
    constexpr auto actual = tokenizer_t("{} services total").count();

    EXPECT_EQ(2, actual.tokens());
    EXPECT_EQ(1, actual.literals());
    EXPECT_EQ(1, actual.placeholders());
}

TEST(Tokenizer, TwoLiteralsWithPlaceholderInTheMiddle) {
    constexpr auto actual = tokenizer_t("exposing {} services total").count();

    EXPECT_EQ(3, actual.tokens());
    EXPECT_EQ(2, actual.literals());
    EXPECT_EQ(1, actual.placeholders());
}

TEST(Tokenizer, Mixed) {
    constexpr auto actual = tokenizer_t("failed to expose service on local endpoint {}:{}: {}").count();

    EXPECT_EQ(6, actual.tokens());
    EXPECT_EQ(3, actual.literals());
    EXPECT_EQ(3, actual.placeholders());
}

TEST(Tokenizer, ThrowsOnGetLiteralEmpty) {
    constexpr auto t = tokenizer_t("");

    EXPECT_THROW(t.literal(0), std::out_of_range);
}

TEST(Tokenizer, GetLiteralSingle) {
    constexpr auto t = tokenizer_t("exposing service on local endpoint");

    EXPECT_EQ(string_view("exposing service on local endpoint"), t.literal(0));
    EXPECT_THROW(t.literal(1), std::out_of_range);
}

TEST(Tokenizer, GetLiteralWithBraceSymbols) {
    constexpr auto t = tokenizer_t("service {{ name: storage }}");

    EXPECT_EQ(string_view("service {"), t.literal(0));
    EXPECT_EQ(string_view(" name: storage }"), t.literal(1));
    EXPECT_THROW(t.literal(2), std::out_of_range);
}

TEST(Tokenizer, GetLiteralWithPlaceholder) {
    constexpr auto t = tokenizer_t("exposing service on local endpoint {}");

    EXPECT_EQ(string_view("exposing service on local endpoint "), t.literal(0));
    EXPECT_THROW(t.literal(1), std::out_of_range);
}

TEST(Tokenizer, GetLiteralWithTwoPlaceholders) {
    constexpr auto t = tokenizer_t("exposing service on local endpoint {}:{}");

    EXPECT_EQ(string_view("exposing service on local endpoint "), t.literal(0));
    EXPECT_EQ(string_view(":"), t.literal(1));
    EXPECT_THROW(t.literal(2), std::out_of_range);
}

TEST(Tokenizer, GetLiteralOfThree) {
    constexpr auto t = tokenizer_t("failed to expose service on local endpoint {}:{}: {}");

    EXPECT_EQ(string_view("failed to expose service on local endpoint "), t.literal(0));
    EXPECT_EQ(string_view(":"), t.literal(1));
    EXPECT_EQ(string_view(": "), t.literal(2));
    EXPECT_THROW(t.literal(3), std::out_of_range);
}

TEST(Tokenizer, GetLiteralOfFour) {
    constexpr auto t = tokenizer_t("remote {} id mismatch: '{}' vs. '{}'");

    EXPECT_EQ(string_view("remote "), t.literal(0));
    EXPECT_EQ(string_view(" id mismatch: '"), t.literal(1));
    EXPECT_EQ(string_view("' vs. '"), t.literal(2));
    EXPECT_EQ(string_view("'"), t.literal(3));
    EXPECT_THROW(t.literal(4), std::out_of_range);
}

TEST(Tokenizer, GetPlaceholderSingle) {
    constexpr auto t = tokenizer_t("exposing service on local endpoint {}");

    EXPECT_EQ(string_view("{}"), t.placeholder(0));
    EXPECT_THROW(t.placeholder(1), std::out_of_range);
}

TEST(Tokenizer, GetPlaceholderOfTwo) {
    constexpr auto t = tokenizer_t("exposing service on local endpoint {}:{}");

    EXPECT_EQ(string_view("{}"), t.placeholder(0));
    EXPECT_EQ(string_view("{}"), t.placeholder(1));
    EXPECT_THROW(t.placeholder(2), std::out_of_range);
}

TEST(Tokenizer, CollectLiterals) {
    constexpr auto t = tokenizer_t("remote {} id mismatch: '{}' vs. '{}'");

    const std::array<string_view, 4> expected = {{
        string_view("remote "),
        string_view(" id mismatch: '"),
        string_view("' vs. '"),
        string_view("'")
    }};

    EXPECT_EQ(expected, (t.literals<4, 7>()));
}

TEST(Tokenizer, CollectLiteralsFourWithBraceSymbols) {
    constexpr auto t = tokenizer_t("remote {} {{id}} mismatch: '{}' vs. '{}'");

    const std::array<string_view, 6> expected = {{
        string_view("remote "),
        string_view(" {"),
        string_view("id}"),
        string_view(" mismatch: '"),
        string_view("' vs. '"),
        string_view("'")
    }};

    EXPECT_EQ(expected, (t.literals<6, 9>()));
}

TEST(Tokenizer, CollectPlaceholders) {
    constexpr auto t = tokenizer_t("remote {} id mismatch: '{}' vs. '{}'");

    const std::array<string_view, 3> expected = {{
        string_view("{}"),
        string_view("{}"),
        string_view("{}")
    }};

    EXPECT_EQ(expected, (t.placeholders<3, 7>()));
}

TEST(TokenGet, GetOfFour) {
    constexpr auto t = tokenizer_t("remote {} {{id}} mismatch: '{}' vs. '{}'");

    EXPECT_EQ(6, t.count().literals());
    EXPECT_EQ(3, t.count().placeholders());

    EXPECT_EQ(token_t::literal("remote "), t.get(0));
    EXPECT_EQ(token_t::placeholder("{}"), t.get(1));
    EXPECT_EQ(token_t::literal(" {"), t.get(2));
    EXPECT_EQ(token_t::literal("id}"), t.get(3));
    EXPECT_EQ(token_t::literal(" mismatch: '"), t.get(4));
    EXPECT_EQ(token_t::placeholder("{}"), t.get(5));
    EXPECT_EQ(token_t::literal("' vs. '"), t.get(6));
    EXPECT_EQ(token_t::placeholder("{}"), t.get(7));
    EXPECT_EQ(token_t::literal("'"), t.get(8));
}

TEST(Tokenizer, Format) {
    constexpr auto t = experimental::detail::tokenizer<6, 11>("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");

    fmt::MemoryWriter wr;
    t.format(wr, "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326);

    EXPECT_EQ(string_view("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326"),
        string_view(wr.data(), wr.size()));
}

// TEST(Pattern, Final) {
//     constexpr auto count = tokenizer_t("{} - {} [{}] 'GET {} HTTP/1.0' {} {}").count();
//     constexpr auto token = tokenizer<count.placeholders(), count.total()>("{} - {} [{}] 'GET {} HTTP/1.0' {} {}");
//
//     EXPECT_EQ(string_view("{} - {} [{}] 'GET {} HTTP/1.0' {} {}"), token.pattern());
//
//     // TODO: EQ literals.
//     // TODO: EQ placeholders.
//
//     fmt::MemoryWriter wr;
//     token.format(wr, "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326);
//
//     EXPECT_EQ(string_view("[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326"),
//         string_view(wr.data(), wr.size()));
// }

}  // namespace testing
}  // namespace blackhole
