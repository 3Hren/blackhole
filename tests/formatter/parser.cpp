#include <gtest/gtest.h>

#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>

#include <blackhole/detail/formatter/string/parser.hpp>

namespace blackhole {
namespace testing {

using detail::formatter::string::broken_t;
using detail::formatter::string::illformed_t;
using detail::formatter::string::invalid_placeholder_t;

using detail::formatter::string::parser_t;

using detail::formatter::string::literal_t;

using detail::formatter::string::num;
using detail::formatter::string::user;

using detail::formatter::string::ph::generic_t;
using detail::formatter::string::ph::leftover_t;
using detail::formatter::string::ph::message_t;
using detail::formatter::string::ph::severity;
using detail::formatter::string::ph::timestamp;

TEST(parser_t, Empty) {
    parser_t parser("");

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, Literal) {
    parser_t parser("literal");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("literal", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, Placeholder) {
    parser_t parser("{id}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<generic_t>(*token).name);
    EXPECT_EQ("{}", boost::get<generic_t>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, PlaceholderWithSpec) {
    parser_t parser("{id:.3f}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<generic_t>(*token).name);
    EXPECT_EQ("{:.3f}", boost::get<generic_t>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, ThrowsExceptionIfPlaceholderIsNotFullyClosed) {
    parser_t parser("{id");
    EXPECT_THROW(parser.next(), illformed_t);
    EXPECT_THROW(parser.next(), broken_t);
}

TEST(parser_t, ThrowsExceptionOnCloseBraceWhileNotInPlaceholderState) {
    parser_t parser("id } is bad");
    EXPECT_THROW(parser.next(), illformed_t);
    EXPECT_THROW(parser.next(), broken_t);
}

TEST(parser_t, ThrowsExceptionIfRequiredPlaceholderHasInvalidSymbols) {
    for (const auto& pattern : {"{id }", "{id-}", "{id+}", "{id=}"}) {
        parser_t parser(pattern);

        EXPECT_THROW(parser.next(), invalid_placeholder_t);
        EXPECT_THROW(parser.next(), broken_t);
    }
}

TEST(parser_t, Braces) {
    parser_t parser("{{}}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{}", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, InverseBraces) {
    parser_t parser("}}{{");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("}{", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, DoubleBraces) {
    parser_t parser("{{{{}}}}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{{}}", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, LiteralWithBraces) {
    parser_t parser("id {{inside}} outside");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id {inside} outside", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, LiteralFollowedByRequiredPlaceholder) {
    parser_t parser("id={id}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id=", boost::get<literal_t>(*token).value);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<generic_t>(*token).name);
    EXPECT_EQ("{}", boost::get<generic_t>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, PlaceholderFollowedByLiteral) {
    parser_t parser("{id} == id");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<generic_t>(*token).name);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ(" == id", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, PlaceholderSurroundedByLiterals) {
    parser_t parser("id={id:<30} definitely an id");

    auto token = parser.next();

    ASSERT_TRUE(!!token);
    EXPECT_EQ("id=", boost::get<literal_t>(*token).value);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("id", boost::get<generic_t>(*token).name);
    EXPECT_EQ("{:<30}", boost::get<generic_t>(*token).spec);

    token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ(" definitely an id", boost::get<literal_t>(*token).value);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, Severity) {
    parser_t parser("{severity}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{}", boost::get<severity<user>>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, SeveritySpec) {
    parser_t parser("{severity:d}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{:d}", boost::get<severity<num>>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, Timestamp) {
    parser_t parser("{timestamp}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{}", boost::get<timestamp<user>>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, TimestampNumeric) {
    parser_t parser("{timestamp:d}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{:d}", boost::get<timestamp<num>>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, TimestampString) {
    parser_t parser("{timestamp:{%Y-%m-%d %H:%M:%S.%f %z}s}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("%Y-%m-%d %H:%M:%S.%f %z", boost::get<timestamp<user>>(*token).pattern);
    EXPECT_EQ("{:s}", boost::get<timestamp<user>>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, Message) {
    parser_t parser("{message}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{}", boost::get<message_t>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, MessageSpec) {
    parser_t parser("{message:<30}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    EXPECT_EQ("{:<30}", boost::get<message_t>(*token).spec);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, Leftover) {
    parser_t parser("{...}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);
    ASSERT_EQ("...", boost::get<leftover_t>(*token).name);

    EXPECT_FALSE(parser.next());
}

TEST(parser_t, LeftoverNamed) {
    parser_t parser("{...last}");

    auto token = parser.next();
    ASSERT_TRUE(!!token);

    ASSERT_EQ("...last", boost::get<leftover_t>(*token).name);

    EXPECT_FALSE(parser.next());
}

}  // namespace testing
}  // namespace blackhole
