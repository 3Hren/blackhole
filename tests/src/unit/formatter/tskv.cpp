#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/algorithm/string/predicate.hpp>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/stdext/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter.hpp>
#include <blackhole/formatter/tskv.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace {

TEST(tskv_t, Format) {
    auto formatter = builder<tskv_t>()
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    std::ostringstream stream;
    // Timestamp are formatted using default value.
    stream << "tskv"
        << "\ttimestamp=" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S %z")
        << "\tseverity=" << 0
        << "\tpid=" << ::getpid()
#ifdef __linux__
        << "\ttid=" << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0')
#else
        << "\ttid=" << std::this_thread::get_id()
#endif
        << "\tmessage=value\n";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(tskv_t, FormatWithInsert) {
    auto formatter = builder<tskv_t>()
        .insert("tskv_format", "cocaine")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    std::ostringstream stream;
    // Timestamp are formatted using default value.
    stream << "tskv"
        << "\ttimestamp=" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S %z")
        << "\tseverity=" << 0
        << "\tpid=" << ::getpid()
#ifdef __linux__
        << "\ttid=" << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0')
#else
        << "\ttid=" << std::this_thread::get_id()
#endif
        << "\tmessage=value"
        << "\ttskv_format=cocaine\n";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

// TODO: Insert constant fields.
// TODO: Escape attribute values.
// TODO: Rename fields.
// TODO: Mutate fields by format.
// TODO: Remove fields by name.

// {
//     "insert": {
//         "tskv_format": "YT",
//         ...
//     },
//     "rename": {
//         "message": "@message",
//         ...
//     },
//     "mutate": {
//         "timestamp": {},
//         "timezone": {
//             "strftime": "%z"
//         },
//         ...
//     },
//     "remove": ["severity"]
// }

} // namespace
} // namespace formatter
} // namespace v1
} // namespace blackhole
