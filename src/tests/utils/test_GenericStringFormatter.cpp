#include "../global.hpp"

#include <blackhole/detail/string/formatting/formatter.hpp>

using namespace blackhole::aux;

TEST(formatter_t, Class) {
    formatter_t formatter("pattern");
    UNUSED(formatter);
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

TEST(formatter_t, FormatOnlyLiteral) {
    formatter_t formatter("literal");
    EXPECT_EQ("literal", formatter.execute(placeholder_action_t()));
}

TEST(formatter_t, FormatOnlyPlaceholder) {
    formatter_t formatter("%(level)s");
    EXPECT_EQ("WARNING", formatter.execute(placeholder_action_t()));
}

TEST(formatter_t, FormatPrefixLiteralWithPlaceholder) {
    formatter_t formatter("Level=%(level)s");
    EXPECT_EQ("Level=WARNING", formatter.execute(placeholder_action_t()));
}

TEST(formatter_t, FormatSuffixLiteralWithPlaceholder) {
    formatter_t formatter("%(level)s is level");
    EXPECT_EQ("WARNING is level", formatter.execute(placeholder_action_t()));
}

TEST(formatter_t, FormatMixedLiteralWithPlaceholder) {
    formatter_t formatter("Level=%(level)s is level");
    EXPECT_EQ("Level=WARNING is level", formatter.execute(placeholder_action_t()));
}

TEST(formatter_t, FormatBrokenPlaceholder) {
    formatter_t formatter("Level=%(level)");
    EXPECT_EQ("Level=%(level)", formatter.execute(placeholder_action_t()));
}
