#include "Mocks.hpp"

TEST(PatternParser, ParseVariadicLocalScopeToken) {
    std::string pattern("%(...L)s");
    auto config = formatter::string::pattern_parser_t::parse(pattern);
    EXPECT_EQ("%s", config.pattern);
    EXPECT_EQ(std::vector<std::string>({ "...0" }), config.attribute_names);
}

TEST(PatternParser, ParseVariadicEventScopeToken) {
    std::string pattern("%(...E)s");
    auto config = formatter::string::pattern_parser_t::parse(pattern);
    EXPECT_EQ("%s", config.pattern);
    EXPECT_EQ(std::vector<std::string>({ "...1" }), config.attribute_names);
}

TEST(PatternParser, ParseVariadicGlobalScopeToken) {
    std::string pattern("%(...G)s");
    auto config = formatter::string::pattern_parser_t::parse(pattern);
    EXPECT_EQ("%s", config.pattern);
    EXPECT_EQ(std::vector<std::string>({ "...2" }), config.attribute_names);
}

TEST(PatternParser, ParseVariadicThreadScopeToken) {
    std::string pattern("%(...T)s");
    auto config = formatter::string::pattern_parser_t::parse(pattern);
    EXPECT_EQ("%s", config.pattern);
    EXPECT_EQ(std::vector<std::string>({ "...3" }), config.attribute_names);
}

TEST(PatternParser, ParseVariadicUniverseScopeToken) {
    std::string pattern("%(...U)s");
    auto config = formatter::string::pattern_parser_t::parse(pattern);
    EXPECT_EQ("%s", config.pattern);
    EXPECT_EQ(std::vector<std::string>({ "...4" }), config.attribute_names);
}
