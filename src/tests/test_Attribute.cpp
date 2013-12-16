#include "Mocks.hpp"

TEST(Attribute, CanMakeCustomAttribute) {
    auto attr = attribute::make<std::int32_t>("custom", 42);
    EXPECT_EQ("custom", attr.first);
    EXPECT_EQ(42, boost::get<std::int32_t>(attr.second.value));
}

TEST(Attribute, TimestampIsEventAttribute) {
    log::attribute_pair_t pair = (keyword::timestamp() = std::time_t(0));
    EXPECT_EQ(log::attribute::scope::event, pair.second.scope);
}

TEST(Attribute, SeverityIsEventAttribute) {
    enum class level { debug };
    log::attribute_pair_t pair = (keyword::severity<level>() = level::debug);
    EXPECT_EQ(log::attribute::scope::event, pair.second.scope);
}

TEST(Attribute, MessageIsEventAttribute) {
    log::attribute_pair_t pair = (keyword::message() = "le message");
    EXPECT_EQ(log::attribute::scope::event, pair.second.scope);
}
