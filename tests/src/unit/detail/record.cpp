/// blackhole/detail/record.hpp

#include <chrono>
#include <string>
#include <thread>

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/apply_visitor.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/record.hpp"
#include "blackhole/severity.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/record.hpp"

namespace blackhole {
inline namespace v1 {

class record_t;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace detail {

struct into_owned_t {
    typedef attribute::value_t result_type;

    template<typename T>
    auto operator()(T value) const -> result_type {
        return {value};
    }

    auto operator()(const attribute::view_t::string_type& value) const -> result_type {
        return {value.to_string()};
    }

    auto operator()(const attribute::view_t::function_type& value) const -> result_type {
        writer_t wr;
        value(wr);
        return {wr.result().to_string()};
    }
};

template<typename T>
class owned;

template<>
class owned<record_t> {
public:
    typedef record_t view_type;

private:
    typedef record_t::inner_t inner_t;

    std::string message;
    string_view message_view;

    std::string formatted;
    string_view formatted_view;

    attributes_t attributes;
    attribute_list attributes_view;
    attribute_pack attributes_pack;

    typedef std::aligned_storage<64>::type storage_type;
    storage_type storage;

public:
    explicit owned(const record_t& record) :
        message(record.message().to_string()),
        message_view(message),
        formatted(record.formatted().to_string()),
        formatted_view(formatted)
    {
        auto& inner = this->inner();
        inner.message = message_view;
        inner.formatted = formatted_view;

        inner.severity = record.severity();
        inner.timestamp = record.timestamp();

        inner.tid = record.tid();

        for (auto list : record.attributes()) {
            for (auto kv : list.get()) {
                attributes.push_back({
                    kv.first.to_string(),
                    boost::apply_visitor(into_owned_t(), kv.second.inner().value)
                });
            }
        }

        for (const auto& attribute : attributes) {
            attributes_view.emplace_back(attribute);
        }

        attributes_pack.emplace_back(attributes_view);

        inner.attributes = attributes_pack;
    }

    auto into_view() const noexcept -> record_t {
        return {inner()};
    }

private:
    auto inner() noexcept -> inner_t& {
        return reinterpret_cast<inner_t&>(storage);
    }

    auto inner() const noexcept -> const inner_t& {
        return reinterpret_cast<const inner_t&>(storage);
    }
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole

/// unit/detail/record.cpp

#include <boost/optional/optional.hpp>

#include <gtest/gtest.h>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace {

TEST(owned, FromRecordMessage) {
    boost::optional<owned<record_t>> result;

    {
        const string_view message("GET");
        const attribute_pack pack;

        const record_t record(0, message, pack);

        result.reset(owned<record_t>(record));
    }

    EXPECT_EQ(string_view("GET"), result->into_view().message());
}

TEST(owned, FromRecordFormattedMessage) {
    boost::optional<owned<record_t>> result;

    {
        const string_view message("GET: {}");
        const attribute_pack pack;

        record_t record(0, message, pack);
        record.activate("GET: 42");

        result.reset(owned<record_t>(record));
    }

    EXPECT_EQ(string_view("GET: 42"), result->into_view().formatted());
}

TEST(owned, FromRecordSeverity) {
    boost::optional<owned<record_t>> result;

    {
        const string_view message("");
        const attribute_pack pack;

        record_t record(42, message, pack);

        result.reset(owned<record_t>(record));
    }

    EXPECT_EQ(42, result->into_view().severity());
}

TEST(owned, FromRecordTimestamp) {
    boost::optional<owned<record_t>> result;

    record_t::clock_type::time_point min = {};
    record_t::clock_type::time_point max = {};
    {
        const string_view message("");
        const attribute_pack pack;

        min = record_t::clock_type::now();
        record_t record(0, message, pack);
        record.activate();
        max = record_t::clock_type::now();

        result.reset(owned<record_t>(record));
    }

    EXPECT_TRUE(min <= result->into_view().timestamp());
    EXPECT_TRUE(max >= result->into_view().timestamp());
}

TEST(owned, FromRecordThreadId) {
    boost::optional<owned<record_t>> result;

    {
        const string_view message("");
        const attribute_pack pack;

        record_t record(0, message, pack);

        result.reset(owned<record_t>(record));
    }

    EXPECT_EQ(::pthread_self(), result->into_view().tid());
}

TEST(owned, FromRecordAttributes) {
    boost::optional<owned<record_t>> result;

    {
        const string_view message("");
        const attribute_list attributes{{"key#1", "value#1"}};
        const attribute_pack pack{attributes};

        record_t record(0, message, pack);

        result.reset(owned<record_t>(record));
    }

    const attribute_list attributes{{"key#1", "value#1"}};

    ASSERT_EQ(1, result->into_view().attributes().size());
    EXPECT_EQ(attributes, result->into_view().attributes().at(0).get());
}

}  // namespace
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
