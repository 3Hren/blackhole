#include <blackhole/record.hpp>

#include "global.hpp"

using namespace blackhole;

TEST(record_t, DefaultConstructor) {
    record_t record;

    EXPECT_FALSE(record);
    EXPECT_FALSE(record.valid());
    EXPECT_TRUE(record.attributes().empty());
}

TEST(record_t, ConversionConstructor) {
    attribute::set_view_t view;
    view.insert(attribute::make("key", "value"));

    record_t record(std::move(view));
    EXPECT_TRUE(record);
    EXPECT_TRUE(record.valid());
    EXPECT_EQ("value", record.extract<std::string>("key"));
}

TEST(record_t, MoveConstructor) {
    attribute::set_view_t view;
    view.insert(attribute::make("key", "value"));

    record_t record(std::move(view));
    record_t other(std::move(record));

    EXPECT_FALSE(record);
    EXPECT_FALSE(record.valid());

    EXPECT_TRUE(other);
    EXPECT_TRUE(other.valid());
    EXPECT_EQ("value", other.extract<std::string>("key"));
}

TEST(record_t, InsertAttribute) {
    record_t record;
    record.insert(attribute::make("key", 42));

    EXPECT_TRUE(record);
    EXPECT_TRUE(record.valid());
    EXPECT_EQ(42, record.extract<int>("key"));

    attribute::set_view_t view;
    view.insert(attribute::make("key", 42));
    EXPECT_TRUE(std::equal(view.begin(), view.end(), record.attributes().begin()));
}

TEST(record, ExtractThrowsExceptionOnWrongName) {
    record_t record;
    EXPECT_THROW(record.extract<int>("key"), std::out_of_range);
}

TEST(record, ExtractThrowsExceptionOnWrongType) {
    record_t record;
    record.insert(attribute::make("key", 42));
    EXPECT_THROW(record.extract<std::string>("key"), boost::bad_get);
}
