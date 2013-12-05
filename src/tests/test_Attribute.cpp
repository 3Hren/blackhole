#include "Mocks.hpp"

TEST(Attribute, CanMakeCustomAttribute) {
    auto attribute = attr::make<std::int32_t>("custom", 42);
    EXPECT_EQ("custom", attribute.first);
    EXPECT_EQ(42, boost::get<std::int32_t>(attribute.second));
}
