#include <memory>

#include <gtest/gtest.h>

#include <blackhole/detail/record.owned.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace {

TEST(owned, FromRecordMessage) {
    std::unique_ptr<owned<record_t>> result;

    {
        const string_view message("GET");
        const attribute_pack pack;

        const record_t record(0, message, pack);

        result.reset(new owned<record_t>(record));
    }

    EXPECT_EQ(string_view("GET"), result->into_view().message());
}

TEST(owned, FromRecordFormattedMessage) {
    std::unique_ptr<owned<record_t>> result;

    {
        const string_view message("GET: {}");
        const attribute_pack pack;
        record_t record(0, message, pack);
        record.activate("GET: 42");

        result.reset(new owned<record_t>(record));
    }

    EXPECT_EQ(string_view("GET: 42"), result->into_view().formatted());
}

TEST(owned, FromRecordSeverity) {
    std::unique_ptr<owned<record_t>> result;

    {
        const string_view message("");
        const attribute_pack pack;
        const record_t record(42, message, pack);

        result.reset(new owned<record_t>(record));
    }

    EXPECT_EQ(42, result->into_view().severity());
}

TEST(owned, FromRecordTimestamp) {
    std::unique_ptr<owned<record_t>> result;

    record_t::clock_type::time_point min = {};
    record_t::clock_type::time_point max = {};
    {
        const string_view message("");
        const attribute_pack pack;

        min = record_t::clock_type::now();
        record_t record(0, message, pack);
        record.activate();
        max = record_t::clock_type::now();

        result.reset(new owned<record_t>(record));
    }

    EXPECT_TRUE(min <= result->into_view().timestamp());
    EXPECT_TRUE(max >= result->into_view().timestamp());
}

TEST(owned, FromRecordThreadId) {
    std::unique_ptr<owned<record_t>> result;

    {
        const string_view message("");
        const attribute_pack pack;
        const record_t record(0, message, pack);

        result.reset(new owned<record_t>(record));
    }

    EXPECT_EQ(::pthread_self(), result->into_view().tid());
}

TEST(owned, FromRecordAttributes) {
    std::unique_ptr<owned<record_t>> result;

    {
        const string_view message("");
        const attribute_list attributes{{"key#1", "value#1"}};
        const attribute_pack pack{attributes};
        const record_t record(0, message, pack);

        result.reset(new owned<record_t>(record));
    }

    const attribute_list attributes{{"key#1", "value#1"}};

    ASSERT_EQ(1, result->into_view().attributes().size());
    EXPECT_EQ(attributes, result->into_view().attributes().at(0).get());
}

}  // namespace
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
