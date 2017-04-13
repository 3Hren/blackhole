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
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %z", &tm);
    std::ostringstream stream;
    // Timestamp are formatted using default value.
    stream << "tskv"
        << "\ttimestamp=" << std::string(buffer, len)
        << "\tseverity=" << 0
        << "\tpid=" << ::getpid()
        << "\ttid="
#ifdef __linux__
        << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0')
#endif
        << std::this_thread::get_id()
        << "\tmessage=value";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(tskv_t, FormatWithAttributes) {
    auto formatter = builder<tskv_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"id", 42}, {"key", "value"}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);

    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream suffix;
    suffix << "\tmessage=value"
        << "\tid=42"
        << "\tkey=value";

    EXPECT_TRUE(boost::ends_with(writer.result().to_string(), suffix.str()));
}

TEST(tskv_t, FormatWithCreate) {
    auto formatter = builder<tskv_t>()
        .create("tskv_format", "cocaine")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %z", &tm);
    std::ostringstream stream;
    // Timestamp are formatted using default value.
    stream << "tskv"
        << "\ttskv_format=cocaine"
        << "\ttimestamp=" << std::string(buffer, len)
        << "\tseverity=" << 0
        << "\tpid=" << ::getpid()
        << "\ttid="
#ifdef __linux__
        << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0')
#endif
        << std::this_thread::get_id()
        << "\tmessage=value";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(tskv_t, Escape) {
    auto formatter = builder<tskv_t>()
        .build();

    const string_view message(
        "\x07""bell"
        "\x08""backspace"
        "\x09""tab"
        "\x0a""line feed"
        "\x0b""vtab"
        "\x0c""form feed"
        "\x0d""carriage return"
        "\x1b""escape"
    );
    const attribute_pack pack;
    record_t record(0, message, pack);

    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream suffix;
    suffix << "message=\\abell\\bbackspace\\ttab\\nline feed\\vvtab\\fform feed\\rcarriage return\\eescape";

    EXPECT_TRUE(boost::ends_with(writer.result().to_string(), suffix.str()));
}

TEST(tskv_t, EscapeKey) {
    auto formatter = builder<tskv_t>()
        .build();

    const string_view message("value");
    const attribute_list attributes{{"with=eq", 42}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);

    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream suffix;
    suffix << "message=value\twith\\=eq=42";

    EXPECT_TRUE(boost::ends_with(writer.result().to_string(), suffix.str()));
}

TEST(tskv_t, FormatTimestampWithTimezone) {
    auto formatter = builder<tskv_t>()
        .timestamp("timestamp", "%Y-%m-%d %H:%M:%S")
        .timestamp("timezone", "%z")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    char buffer[128];
    auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    std::ostringstream stream;
    stream << "\ttimestamp=" << std::string(buffer, len);
    len = std::strftime(buffer, sizeof(buffer), "%z", &tm);
    stream << "\ttimezone=" << std::string(buffer, len);

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_TRUE(boost::contains(writer.result().to_string(), stream.str()));
}

TEST(tskv_t, Rename) {
    auto formatter = builder<tskv_t>()
        .rename("message", "@message")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);

    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream suffix;
    suffix << "@message=value";

    EXPECT_TRUE(boost::ends_with(writer.result().to_string(), suffix.str()));
}

TEST(tskv_t, Remove) {
    auto formatter = builder<tskv_t>()
        .remove("message")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_FALSE(boost::contains(writer.result().to_string(), "message"));
    EXPECT_FALSE(boost::contains(writer.result().to_string(), "message=value"));
}

TEST(tskv_t, FormatTimestampWithTimezoneUsingLocaltime) {
    auto formatter = builder<tskv_t>()
        .timestamp("timestamp", "%Y-%m-%d %H:%M:%S", false)
        .timestamp("timezone", "%z", false)
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::localtime_r(&time, &tm);
    char buffer[128];
    auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    std::ostringstream stream;
    stream << "\ttimestamp=" << std::string(buffer, len);
    len = std::strftime(buffer, sizeof(buffer), "%z", &tm);
    stream << "\ttimezone=" << std::string(buffer, len);

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_TRUE(boost::contains(writer.result().to_string(), stream.str()));
}

} // namespace
} // namespace formatter
} // namespace v1
} // namespace blackhole
