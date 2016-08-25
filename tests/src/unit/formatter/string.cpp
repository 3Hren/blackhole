#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/algorithm/string/predicate.hpp>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/stdext/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/record.hpp>

namespace {

struct version_t {
    int major;
    int minor;
};

}  // namespace

namespace blackhole {
inline namespace v1 {

template<>
struct display_traits<version_t> {
    static auto apply(const version_t& version, writer_t& wr) -> void {
        wr.write("{}.{}", version.major, version.minor);
    }
};

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace {

using ::testing::AnyOf;
using ::testing::Eq;

TEST(string_t, Message) {
    auto formatter = builder<string_t>("[{message}]")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[value]", writer.result().to_string());
}

TEST(string_t, Severity) {
    // NOTE: No severity mapping provided, formatter falls back to the numeric case.
    auto formatter = builder<string_t>("[{severity}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[0]", writer.result().to_string());
}

TEST(string_t, SeverityNum) {
    auto formatter = builder<string_t>("[{severity:d}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[0]", writer.result().to_string());
}

TEST(string_t, SeverityUser) {
    auto formatter = builder<string_t>("[{severity}]")
        .mapping([](int, const std::string&, writer_t& writer) {
            writer.write("DEBUG");
        })
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[DEBUG]", writer.result().to_string());
}

TEST(string_t, SeverityUserExplicit) {
    auto formatter = builder<string_t>("[{severity:s}]")
        .mapping([](int, const std::string&, writer_t& writer) {
            writer.write("DEBUG");
        })
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[DEBUG]", writer.result().to_string());
}

TEST(string_t, SeverityNumWithMappingProvided) {
    auto formatter = builder<string_t>("[{severity:d}]")
        .mapping([](int, const std::string&, writer_t& writer) {
            writer.write("DEBUG");
        })
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[0]", writer.result().to_string());
}

TEST(string_t, SeverityNumWithSpec) {
    auto formatter = builder<string_t>("[{severity:*^3d}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[*0*]", writer.result().to_string());
}

TEST(string_t, SeverityUserWithSpec) {
    auto formatter = builder<string_t>("[{severity:<7}]")
        .mapping([](int severity, const std::string& spec, writer_t& writer) {
            writer.write(spec, "DEBUG");
        })
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[DEBUG  ]", writer.result().to_string());
}

TEST(string_t, CombinedSeverityNumWithMessage) {
    auto formatter = builder<string_t>("[{severity:d}]: {message}")
        .build();

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[0]: value", writer.result().to_string());
}

TEST(string_t, Generic) {
    auto formatter = builder<string_t>("{protocol}/{version:.1f}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", {1.1}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("HTTP/1.1", writer.result().to_string());
}

TEST(string_t, GenericNull) {
    auto formatter = builder<string_t>("{protocol}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"protocol", {}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("none", writer.result().to_string());
}

TEST(string_t, GenericNullWithSpec) {
    auto formatter = builder<string_t>("{protocol:>5}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"protocol", {}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(" none", writer.result().to_string());
}

TEST(string_t, GenericDuplicated) {
    auto formatter = builder<string_t>("{protocol}/{version}")
        .build();

    const string_view message("-");
    const attribute_list wrapper{{"protocol", {"HTTP"}}, {"version", {1.1}}};
    const attribute_list scoped{{"protocol", {"FTP"}}, {"version", {42}}};
    const attribute_list user{{"protocol", {"TCP"}}, {"version", {1}}};
    const attribute_pack pack{user, scoped, wrapper};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("TCP/1", writer.result().to_string());
}

TEST(string_t, ThrowsIfGenericAttributeNotFound) {
    auto formatter = builder<string_t>("{protocol}/{version:.1f}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;

    EXPECT_THROW(formatter->format(record, writer), std::logic_error);
}

TEST(string_t, GenericOptionalWhenNotFound) {
    auto formatter = builder<string_t>("{trace:{42:default}#06x}/{span:#06x}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"span", 0}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("0x002a/0x0000", writer.result().to_string());
}

TEST(string_t, GenericLazyUnspec) {
    auto formatter = builder<string_t>("{protocol}/{version}")
        .build();

    version_t version{1, 1};

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", version}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("HTTP/1.1", writer.result().to_string());
}

TEST(string_t, GenericLazySpec) {
    auto formatter = builder<string_t>("{protocol}/{version:>4s} - alpha")
        .build();

    version_t version{1, 1};

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", version}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("HTTP/1.1 - alpha", writer.result().to_string());
}

TEST(string_t, Process) {
    auto formatter = builder<string_t>("{process}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(std::to_string(::getpid()), writer.result().to_string());
}

TEST(string_t, ProcessIdExplicitly) {
    auto formatter = builder<string_t>("{process:d}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(std::to_string(::getpid()), writer.result().to_string());
}

TEST(string_t, ProcessName) {
    auto formatter = builder<string_t>("{process:s}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_TRUE(writer.result().to_string().size() > 0);
}

TEST(string_t, Thread) {
    auto formatter = builder<string_t>("{thread}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream stream;
#ifdef __linux
    stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
    stream << std::this_thread::get_id();

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(string_t, ThreadExplicitly) {
    auto formatter = builder<string_t>("{thread:#x}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream stream;
#ifdef __linux
    stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
    stream << std::this_thread::get_id();

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(string_t, ThreadExplicitlySpec) {
    auto formatter = builder<string_t>("{thread:>#16x}")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    std::ostringstream stream;
#ifdef __linux__
    stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
    stream << std::this_thread::get_id();

    EXPECT_TRUE(boost::ends_with(writer.result().to_string(), stream.str()));
}

struct threadname_guard {
    template<std::size_t N>
    threadname_guard(const char(&name)[N]) {
        #ifdef __linux__
            ::pthread_setname_np(::pthread_self(), name);
        #elif __APPLE__
            ::pthread_setname_np(name);
        #else
        #error "Not implemented. Please write an implementation for your OS."
        #endif
    }

    ~threadname_guard() {
        #ifdef __linux__
            ::pthread_setname_np(::pthread_self(), "");
        #elif __APPLE__
            ::pthread_setname_np("");
        #else
        #error "Not implemented. Please write an implementation for your OS."
        #endif
    }
};

TEST(string_t, ThreadName) {
    auto formatter = builder<string_t>("{thread:s}")
        .build();

    threadname_guard guard("thread#0");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("thread#0", writer.result().to_string());
}

TEST(string_t, ThreadNameWithSpec) {
    auto formatter = builder<string_t>("{thread:>10s}")
        .build();

    threadname_guard guard("thread#0");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("  thread#0", writer.result().to_string());
}

TEST(string_t, Timestamp) {
    // NOTE: By default %Y-%m-%d %H:%M:%S.%f pattern is used.
    auto formatter = builder<string_t>("[{timestamp}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    fmt::MemoryWriter wr;
    wr << "[" << fmt::StringRef(buffer, len) << "." << fmt::pad(std::chrono::duration_cast<
        std::chrono::microseconds
    >(timestamp.time_since_epoch()).count() % 1000000, 6, '0') << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampExplicit) {
    auto formatter = builder<string_t>("[{timestamp:{%Y}}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y", &tm);
    fmt::MemoryWriter wr;
    wr << "[" << fmt::StringRef(buffer, len) << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampExplicitWithType) {
    auto formatter = builder<string_t>("[{timestamp:{%H:%M:%S}s}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    fmt::MemoryWriter wr;
    wr << "[" << fmt::StringRef(buffer, len) << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampSpec) {
    auto formatter = builder<string_t>("[{timestamp:>30s}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::gmtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    fmt::MemoryWriter wr;
    wr << "[    " << fmt::StringRef(buffer, len) << "." << fmt::pad(std::chrono::duration_cast<
        std::chrono::microseconds
    >(timestamp.time_since_epoch()).count() % 1000000, 6, '0') << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampLocaltime) {
    // NOTE: By default %Y-%m-%d %H:%M:%S.%f pattern is used.
    auto formatter = builder<string_t>("[{timestamp:l}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::localtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    fmt::MemoryWriter wr;
    wr << "[" << fmt::StringRef(buffer, len) << "." << fmt::pad(std::chrono::duration_cast<
        std::chrono::microseconds
    >(timestamp.time_since_epoch()).count() % 1000000, 6, '0') << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampExplicitWithTypeLocaltime) {
    auto formatter = builder<string_t>("[{timestamp:{%H:%M:%S}l}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::localtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    fmt::MemoryWriter wr;
    wr << "[" << fmt::StringRef(buffer, len) << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampSpecLocaltime) {
    auto formatter = builder<string_t>("[{timestamp:>30l}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto time = record_t::clock_type::to_time_t(timestamp);
    std::tm tm;
    ::localtime_r(&time, &tm);
    char buffer[128];
    const auto len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    fmt::MemoryWriter wr;
    wr << "[    " << fmt::StringRef(buffer, len) << "." << fmt::pad(std::chrono::duration_cast<
        std::chrono::microseconds
    >(timestamp.time_since_epoch()).count() % 1000000, 6, '0') << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampNum) {
    auto formatter = builder<string_t>("[{timestamp:d}]")
        .build();

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    record.activate();

    const auto timestamp = record.timestamp();
    const auto usec = std::chrono::duration_cast<
        std::chrono::microseconds
    >(timestamp.time_since_epoch()).count();
    fmt::MemoryWriter wr;
    wr << "[" << usec << "]";

    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, Leftover) {
    auto formatter = builder<string_t>("{...}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("key#1: 42, key#2: value#2"),
        Eq("key#2: value#2, key#1: 42")
    ));
}

TEST(string_t, LeftoverEmpty) {
    auto formatter = builder<string_t>("[{...}]")
        .build();

    const string_view message("-");
    const attribute_list attributes{};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("[]", writer.result().to_string());
}

TEST(DISABLED_string_t, LeftoverWithPrefixAndSuffix) {
    auto formatter = builder<string_t>("{...:{[:px}{]:sx}s}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("[key#1: 42, key#2: value#2]"),
        Eq("[key#2: value#2, key#1: 42]")
    ));
}

TEST(DISABLED_string_t, LeftoverEmptyWithPrefixAndSuffix) {
    auto formatter = builder<string_t>("{...:{[:px}{]:sx}s}")
        .build();

    const string_view message("-");
    const attribute_list attributes{};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_EQ("", writer.result().to_string());
}

TEST(string_t, LeftoverWithPattern) {
    auto formatter = builder<string_t>("{...:{{name}={value}:p}s}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("key#1=42, key#2=value#2"),
        Eq("key#2=value#2, key#1=42")
    ));
}

TEST(string_t, LeftoverWithSeparator) {
    auto formatter = builder<string_t>("{...:{ | :s}s}")
        .build();

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter->format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("key#1: 42 | key#2: value#2"),
        Eq("key#2: value#2 | key#1: 42")
    ));
}

TEST(string_t, FactoryType) {
    EXPECT_EQ(std::string("string"), factory<string_t>().type());
}

}  // namespace
}  // namespace formatter
}  // namespace v1
}  // namespace blackhole
