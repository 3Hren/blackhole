#include "../global.hpp"

#include <blackhole/detail/stream/stream.hpp>

using namespace blackhole;

class formatter_t {
    typedef std::function<void(aux::attachable_ostringstream&, const std::string&)> action_type;
    typedef std::function<void(aux::attachable_ostringstream&)> bound_action_type;

    const std::string pattern;
    action_type on_placeholder;

    std::vector<bound_action_type> actions;

    struct literal_action_t {
        std::string literal;
        void operator()(aux::attachable_ostringstream& stream) const {
            stream.flush();
            stream.rdbuf()->storage()->append(literal);
        }
    };

    struct placeholder_action_t {
        action_type action;
        std::string placeholder;
        void operator()(aux::attachable_ostringstream& stream) const {
            stream.flush();
            action(stream, placeholder);
        }
    };
public:
    formatter_t(const std::string& pattern, action_type on_placeholder = action_type()) :
        pattern(pattern),
        on_placeholder(on_placeholder)
    {
        auto it = pattern.begin();
        auto end = pattern.end();

        std::string literal;
        literal.reserve(pattern.length());
        while (it != end) {
            if (*it == '%' && it + 1 != end && *(it + 1) == '(') { // placeholder_begin -> equal({'%', '('})
                it += 2;

                std::string placeholder;
                bool found = false;
                while (it != end) {
                    if (*it == ')' && it + 1 != end && *(it + 1) == 's') {
                        found = true;
                        it += 2;
                        break;
                    }
                    placeholder.push_back(*it);
                    it++;
                }

                if (found && !placeholder.empty()) {
                    if (!literal.empty()) {
                        actions.push_back(literal_action_t{ literal });
                        literal.clear();
                    }

                    actions.push_back(placeholder_action_t{ on_placeholder, placeholder });
                } else {
                    actions.push_back(literal_action_t{ literal + "%(" + placeholder });
                    literal.clear();
                }
            }

            if (it == end) {
                break;
            }
            literal.push_back(*it);
            it++;
        }

        if (!literal.empty()) {
            actions.push_back(literal_action_t{ literal });
            literal.clear();
        }
    }

    std::string execute() {
        std::string buffer;
        aux::attachable_ostringstream stream;
        stream.attach(buffer);
        for (auto it = actions.begin(); it != actions.end(); ++it) {
            const bound_action_type& action = *it;
            action(stream);
        }
        return buffer;
    }
};

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
    void operator()(aux::attachable_ostringstream& stream, const std::string& placeholder) const {
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
