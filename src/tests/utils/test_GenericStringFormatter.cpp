#include "../global.hpp"

#include <blackhole/detail/string/formatting/formatter.hpp>

using namespace blackhole::aux;

TEST(formatter_t, Class) {
    formatter_t formatter("pattern");
    UNUSED(formatter);
}

TEST(formatter_t, FormatOnlyLiteral) {
    formatter_t formatter("literal");
    EXPECT_EQ("literal", formatter.execute());
}

namespace testing {

struct placeholder_action_t {
    void operator()(blackhole::aux::attachable_ostringstream& stream, const std::string& placeholder) const {
        if (placeholder == "level") {
            stream << "WARNING";
        }
    }
};

} // namespace testing

TEST(formatter_t, FormatOnlyPlaceholder) {
    formatter_t formatter("%(level)s", placeholder_action_t());
    EXPECT_EQ("WARNING", formatter.execute());
}

TEST(formatter_t, FormatPrefixLiteralWithPlaceholder) {
    formatter_t formatter("Level=%(level)s", placeholder_action_t());
    EXPECT_EQ("Level=WARNING", formatter.execute());
}

TEST(formatter_t, FormatSuffixLiteralWithPlaceholder) {
    formatter_t formatter("%(level)s is level", placeholder_action_t());
    EXPECT_EQ("WARNING is level", formatter.execute());
}

TEST(formatter_t, FormatMixedLiteralWithPlaceholder) {
    formatter_t formatter("Level=%(level)s is level", placeholder_action_t());
    EXPECT_EQ("Level=WARNING is level", formatter.execute());
}

TEST(formatter_t, FormatBrokenPlaceholder) {
    formatter_t formatter("Level=%(level)", placeholder_action_t());
    EXPECT_EQ("Level=%(level)", formatter.execute());
}
