/// blackhole/detail/record.hpp

#include <chrono>
#include <string>
#include <thread>

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"
#include "blackhole/record.hpp"
#include "blackhole/severity.hpp"

namespace blackhole {
inline namespace v1 {

class record_t;

}
}

namespace blackhole {
inline namespace v1 {
namespace detail {

template<typename T>
class owned;

template<>
class owned<record_t> {
public:
    typedef record_t view_type;

private:
    std::string message;
    string_view message_view;

    std::string formatted;
    string_view formatted_view;


    severity_t severity;
    view_type::time_point timestamp;

    std::thread::native_handle_type tid;

    attributes_t attributes;
    attribute_pack attributes_view;

public:
    explicit owned(const record_t& record) :
        message(record.message().to_string()),
        message_view(message),
        severity(record.severity())
    {
        // TODO: Implement.
    }

    auto into_view() const noexcept -> record_t {
        // TODO: Implement.
        return record_t(0, message_view, attributes_view);
    }
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole

/// unit/detail/record.cpp

#include <gtest/gtest.h>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace {

TEST(owned, FromRecordMessage) {
    const string_view message("GET");
    const attribute_pack pack;

    const record_t record(0, message, pack);
    owned<record_t> owned(record);

    EXPECT_EQ(string_view("GET"), owned.into_view().message());
}

}  // namespace
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
