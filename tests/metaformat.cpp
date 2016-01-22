#include <gtest/gtest.h>

#include "blackhole/extensions/format.hpp"
#include <blackhole/extensions/metaformat2.hpp>

namespace blackhole {
namespace testing {

using namespace blackhole::experimental;

using blackhole::experimental::detail::collect;
using blackhole::experimental::detail::count;
using blackhole::experimental::detail::get;
using blackhole::experimental::detail::pattern;
using blackhole::experimental::detail::tokenizer;
using blackhole::experimental::detail::tokenizer_t;

TEST(CountOfLiteral, Empty) {
    constexpr auto actual = count<literal_t>("");
    EXPECT_EQ(1, actual);
}

TEST(CountOfLiteral, Single) {
    constexpr auto actual = count<literal_t>("exposing service on local endpoint");
    EXPECT_EQ(1, actual);
}

TEST(CountOfLiteral, SingleWithBraceSymbols) {
    constexpr auto actual = count<literal_t>("service {{ name: storage }}");
    EXPECT_EQ(1, actual);
}

TEST(CountOfLiteral, WithSinglePlaceholderAtTheEnd) {
    constexpr auto actual = count<literal_t>("exposing service on local endpoint {}");
    EXPECT_EQ(1, actual);
}

TEST(CountOfLiteral, WithSinglePlaceholderAtTheBeginning) {
    constexpr auto actual = count<literal_t>("{} services total");
    EXPECT_EQ(1, actual);
}

TEST(CountOfLiteral, TwoLiterals) {
    constexpr auto actual = count<literal_t>("exposing {} services total");
    EXPECT_EQ(2, actual);
}

TEST(CountOfLiteral, ThreeLiterals) {
    constexpr auto actual = count<literal_t>("failed to expose service on local endpoint {}:{}: {}");
    EXPECT_EQ(3, actual);
}

TEST(CountOfPlaceholders, Empty) {
    constexpr auto actual = count<placeholder_t>("");
    EXPECT_EQ(0, actual);
}

TEST(CountOfPlaceholders, WithBraceSymbols) {
    constexpr auto actual = count<placeholder_t>("service {{ name: storage }}");
    EXPECT_EQ(0, actual);
}

TEST(CountOfPlaceholders, Single) {
    constexpr auto actual = count<placeholder_t>("{}");
    EXPECT_EQ(1, actual);
}

TEST(CountOfPlaceholders, WithSinglePlaceholderAtTheEnd) {
    constexpr auto actual = count<placeholder_t>("exposing service on local endpoint {}");
    EXPECT_EQ(1, actual);
}

TEST(CountOfPlaceholders, Two) {
    constexpr auto actual = count<placeholder_t>("{}{}");
    EXPECT_EQ(2, actual);
}

TEST(CountOfPlaceholders, WithSinglePlaceholderAtTheBeginning) {
    constexpr auto actual = count<placeholder_t>("{} services total");
    EXPECT_EQ(1, actual);
}

TEST(CountOfPlaceholders, TwoLiterals) {
    constexpr auto actual = count<placeholder_t>("exposing {} services total");
    EXPECT_EQ(1, actual);
}

TEST(CountOfPlaceholders, Three) {
    constexpr auto actual = count<placeholder_t>("failed to expose service on local endpoint {}:{}: {}");
    EXPECT_EQ(3, actual);
}

TEST(GetLiteral, Empty) {
    constexpr auto actual = get<literal_t>("", 0);
    EXPECT_EQ(string_view(""), actual.get());
}

TEST(GetLiteral, Single) {
    constexpr auto actual = get<literal_t>("exposing service on local endpoint", 0);
    EXPECT_EQ(string_view("exposing service on local endpoint"), actual.get());
}

TEST(GetLiteral, SingleWithBraceSymbols) {
    constexpr auto actual = get<literal_t>("service {{ name: storage }}", 0);
    EXPECT_EQ(string_view("service {{ name: storage }}"), actual.get());
}

TEST(GetLiteral, GetOneOfOne) {
    constexpr auto actual = get<literal_t>("exposing service on local endpoint {}", 0);
    EXPECT_EQ(string_view("exposing service on local endpoint "), actual.get());
}

TEST(GetLiteral, GetOfTwo) {
    constexpr auto a0 = get<literal_t>("exposing service on local endpoint {}:{}", 0);
    EXPECT_EQ(string_view("exposing service on local endpoint "), a0.get());

    constexpr auto a1 = get<literal_t>("exposing service on local endpoint {}:{}", 1);
    EXPECT_EQ(string_view(":"), a1.get());
}

TEST(GetLiteral, GetOfThree) {
    constexpr auto a0 = get<literal_t>("failed to expose service on local endpoint {}:{}: {}", 0);
    EXPECT_EQ(string_view("failed to expose service on local endpoint "), a0.get());

    constexpr auto a1 = get<literal_t>("failed to expose service on local endpoint {}:{}: {}", 1);
    EXPECT_EQ(string_view(":"), a1.get());

    constexpr auto a2 = get<literal_t>("failed to expose service on local endpoint {}:{}: {}", 2);
    EXPECT_EQ(string_view(": "), a2.get());
}

TEST(GetLiteral, GetOfFour) {
    constexpr auto a0 = get<literal_t>("remote {} id mismatch: '{}' vs. '{}'", 0);
    EXPECT_EQ(string_view("remote "), a0.get());

    constexpr auto a1 = get<literal_t>("remote {} id mismatch: '{}' vs. '{}'", 1);
    EXPECT_EQ(string_view(" id mismatch: '"), a1.get());

    constexpr auto a2 = get<literal_t>("remote {} id mismatch: '{}' vs. '{}'", 2);
    EXPECT_EQ(string_view("' vs. '"), a2.get());

    constexpr auto a3 = get<literal_t>("remote {} id mismatch: '{}' vs. '{}'", 3);
    EXPECT_EQ(string_view("'"), a3.get());
}

TEST(GetPlaceholder, Single) {
    constexpr auto actual = get<placeholder_t>("exposing service on local endpoint {}", 0);
    EXPECT_EQ(string_view("{}"), actual.get());
}

TEST(GetPlaceholder, GetOfTwo) {
    constexpr auto a0 = get<placeholder_t>("exposing service on local endpoint {}:{}", 0);
    EXPECT_EQ(string_view("{}"), a0.get());

    constexpr auto a1 = get<placeholder_t>("exposing service on local endpoint {}:{}", 1);
    EXPECT_EQ(string_view("{}"), a1.get());
}

TEST(CollectLiteral, Four) {
    constexpr auto n = count<literal_t>("remote {} id mismatch: '{}' vs. '{}'");
    constexpr auto c = collect<literal_t, n>("remote {} id mismatch: '{}' vs. '{}'");

    const std::array<literal_t, 4> expected = {
        literal_t{"remote "},
        literal_t{" id mismatch: '"},
        literal_t{"' vs. '"},
        literal_t{"'"}
    };

    EXPECT_EQ(expected, c);
}

TEST(CollectLiteral, FourWithBraceSymbols) {
    constexpr auto c = collect<literal_t, 4>("remote {} {{id}} mismatch: '{}' vs. '{}'");

    const std::array<literal_t, 4> expected = {
        literal_t{"remote "},
        literal_t{" {{id}} mismatch: '"},
        literal_t{"' vs. '"},
        literal_t{"'"}
    };

    EXPECT_EQ(expected, c);
}

TEST(CollectPlaceholders, Three) {
    constexpr auto c = collect<placeholder_t, 3>("remote {} id mismatch: '{}' vs. '{}'");

    const std::array<placeholder_t, 3> expected = {
        placeholder_t{"{}"},
        placeholder_t{"{}"},
        placeholder_t{"{}"}
    };

    EXPECT_EQ(expected, c);
}

TEST(Pattern, Pattern) {
    constexpr auto p = pattern<4, 3>("remote {} id mismatch: '{}' vs. '{}'");

    const std::array<literal_t, 4> literals = {
        literal_t{"remote "},
        literal_t{" id mismatch: '"},
        literal_t{"' vs. '"},
        literal_t{"'"}
    };

    const std::array<placeholder_t, 3> placeholders = {
        placeholder_t{"{}"},
        placeholder_t{"{}"},
        placeholder_t{"{}"}
    };

    EXPECT_EQ(string_view("remote {} id mismatch: '{}' vs. '{}'"), p.get());
    EXPECT_EQ(literals, p.literals());
    EXPECT_EQ(placeholders, p.placeholders());
}

TEST(TokenCount, Empty) {
    constexpr auto actual = tokenizer_t("").count();
    const std::tuple<std::size_t, std::size_t> expected(0, 0);
    EXPECT_EQ(expected, actual);
}

TEST(TokenCount, SingleLiteral) {
    // Result: ["exposing service on local endpoint"].

    constexpr auto actual = tokenizer_t("exposing service on local endpoint").count();
    const std::tuple<std::size_t, std::size_t> expected(1, 0);
    EXPECT_EQ(expected, actual);
}

TEST(TokenCount, SingleWithBraceSymbols) {
    // Result: ["service {", " name: storage }"].

    constexpr auto actual = tokenizer_t("service {{ name: storage }}").count();
    const std::tuple<std::size_t, std::size_t> expected(2, 0);
    EXPECT_EQ(expected, actual);
}

TEST(TokenCount, SingleWithBraceSymbolsNotInTheEnd) {
    // Result: ["service {", " name: storage }", "."].

    constexpr auto actual = tokenizer_t("service {{ name: storage }}.").count();
    const std::tuple<std::size_t, std::size_t> expected(3, 0);
    EXPECT_EQ(expected, actual);
}

TEST(TokenCount, SinglePlaceholder) {
    constexpr auto actual = tokenizer_t("{}").count();
    const std::tuple<std::size_t, std::size_t> expected(0, 1);
    EXPECT_EQ(expected, actual);
}

TEST(TokenCount, SinglePlaceholderAtTheEnd) {
    constexpr auto actual = tokenizer_t("exposing service on local endpoint {}").count();
    const std::tuple<std::size_t, std::size_t> expected(1, 1);
    EXPECT_EQ(expected, actual);
}

TEST(TokenCount, TwoLiteralsOnePlaceholder) {
    constexpr auto actual = tokenizer_t("exposing {} services total").count();
    const std::tuple<std::size_t, std::size_t> expected(2, 1);
    EXPECT_EQ(expected, actual);
}

TEST(TokenGet, GetOfFour) {
    constexpr auto t = tokenizer_t("remote {} {{id}} mismatch: '{}' vs. '{}'");

    const std::tuple<std::size_t, std::size_t> expected(6, 3);
    EXPECT_EQ(expected, t.count());

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

}  // namespace testing
}  // namespace blackhole
