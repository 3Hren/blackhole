#include <blackhole/attribute.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/severity.hpp>
#include <blackhole/keyword/timestamp.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(Attribute, CanMakeCustomAttribute) {
    auto attr = attribute::make<std::int32_t>("custom", 42);
    EXPECT_EQ("custom", attr.first);
    EXPECT_EQ(42, boost::get<std::int32_t>(attr.second.value));
}

TEST(Attribute, TimestampIsEventAttribute) {
    attribute_pair_t pair = (keyword::timestamp() = timeval{ 0, 0 });
    EXPECT_EQ(attribute::scope::event, pair.second.scope);
}

TEST(Attribute, SeverityIsEventAttribute) {
    attribute_pair_t pair = (keyword::severity<testing::level>() = testing::level::debug);
    EXPECT_EQ(attribute::scope::event, pair.second.scope);
}

TEST(Attribute, MessageIsEventAttribute) {
    attribute_pair_t pair = (keyword::message() = "le message");
    EXPECT_EQ(attribute::scope::event, pair.second.scope);
}
