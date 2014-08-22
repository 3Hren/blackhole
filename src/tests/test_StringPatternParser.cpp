#include <blackhole/formatter/string/builder.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(PatternParser, ParseVariadicLocalScopeToken) {
    std::string pattern("%(...L)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...1"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}

TEST(PatternParser, ParseVariadicEventScopeToken) {
    std::string pattern("%(...E)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...2"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}

TEST(PatternParser, ParseVariadicGlobalScopeToken) {
    std::string pattern("%(...G)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...4"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}

TEST(PatternParser, ParseVariadicThreadScopeToken) {
    std::string pattern("%(...T)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...8"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}

TEST(PatternParser, ParseVariadicUniverseScopeToken) {
    std::string pattern("%(...U)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...16"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}

TEST(PatternParser, ParseVariadicMixedScopeTokens) {
    std::string pattern("%(...LE)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...3"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}

TEST(PatternParser, SquishScopes) {
    std::string pattern("%(...LLLUUE)s");
    std::string::const_iterator it = pattern.begin();
    EXPECT_EQ(std::make_tuple(std::string("...19"), formatter::string::variadic),
              formatter::string::aux::extract_key(it, pattern.end()));
}
