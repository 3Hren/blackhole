#include "../global.hpp"

class formatter_t {
    const std::string pattern;
public:
    formatter_t(const std::string& pattern) :
        pattern(pattern)
    {}
};

TEST(formatter_t, Class) {
    formatter_t formatter("pattern");
    UNUSED(formatter);
}
