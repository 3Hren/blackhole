#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/algorithm/string/predicate.hpp>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
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
namespace testing {

using ::testing::AnyOf;
using ::testing::Eq;

TEST(string_t, Message) {
    formatter::string_t formatter("[{message}]");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[value]", writer.result().to_string());
}

TEST(string_t, Severity) {
    // NOTE: No severity mapping provided, formatter falls back to the numeric case.
    formatter::string_t formatter("[{severity}]");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]", writer.result().to_string());
}

TEST(string_t, SeverityNum) {
    formatter::string_t formatter("[{severity:d}]");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]", writer.result().to_string());
}

TEST(string_t, SeverityUser) {
    formatter::string_t formatter("[{severity}]", [](int severity, const std::string&, writer_t& writer) {
        writer.write("DEBUG");
    });

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[DEBUG]", writer.result().to_string());
}

TEST(string_t, SeverityUserExplicit) {
    formatter::string_t formatter("[{severity:s}]", [](int severity, const std::string&, writer_t& writer) {
        writer.write("DEBUG");
    });

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[DEBUG]", writer.result().to_string());
}

TEST(string_t, SeverityNumWithMappingProvided) {
    formatter::string_t formatter("[{severity:d}]", [](int severity, const std::string&, writer_t& writer) {
        writer.write("DEBUG");
    });

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]", writer.result().to_string());
}

TEST(string_t, SeverityNumWithSpec) {
    formatter::string_t formatter("[{severity:*^3d}]");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[*0*]", writer.result().to_string());
}

TEST(string_t, SeverityUserWithSpec) {
    formatter::string_t formatter("[{severity:<7}]", [](int severity, const std::string& spec, writer_t& writer) {
        writer.write(spec, "DEBUG");
    });

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[DEBUG  ]", writer.result().to_string());
}

TEST(string_t, CombinedSeverityNumWithMessage) {
    formatter::string_t formatter("[{severity:d}]: {message}");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]: value", writer.result().to_string());
}

TEST(string_t, Generic) {
    formatter::string_t formatter("{protocol}/{version:.1f}");

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", {1.1}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("HTTP/1.1", writer.result().to_string());
}

TEST(string_t, GenericNull) {
    formatter::string_t formatter("{protocol}");

    const string_view message("-");
    const attribute_list attributes{{"protocol", {}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("none", writer.result().to_string());
}

TEST(string_t, GenericNullWithSpec) {
    formatter::string_t formatter("{protocol:>5}");

    const string_view message("-");
    const attribute_list attributes{{"protocol", {}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ(" none", writer.result().to_string());
}

TEST(string_t, ThrowsIfGenericAttributeNotFound) {
    formatter::string_t formatter("{protocol}/{version:.1f}");

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;

    EXPECT_THROW(formatter.format(record, writer), std::logic_error);
}

TEST(string_t, GenericOptional) {
    formatter::string_t formatter("{protocol}{version:.1f}");
    formatter.optional("version", "/", " - REQUIRED");

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", {1.1}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("HTTP/1.1 - REQUIRED", writer.result().to_string());
}

TEST(string_t, GenericLazyUnspec) {
    formatter::string_t formatter("{protocol}/{version}");

    version_t version{1, 1};

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", version}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("HTTP/1.1", writer.result().to_string());
}

TEST(string_t, GenericLazySpec) {
    formatter::string_t formatter("{protocol}/{version:>4s} - alpha");

    version_t version{1, 1};

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", version}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("HTTP/1.1 - alpha", writer.result().to_string());
}

TEST(string_t, ThrowsIfOptionsContainsReservedPlaceholderNames) {
    formatter::string_t formatter("{protocol}");

    EXPECT_THROW(formatter.optional("message", "[", "]"), std::logic_error);
}

TEST(string_t, Process) {
    formatter::string_t formatter("{process}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ(std::to_string(::getpid()), writer.result().to_string());
}

TEST(string_t, ProcessIdExplicitly) {
    formatter::string_t formatter("{process:d}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ(std::to_string(::getpid()), writer.result().to_string());
}

TEST(string_t, ProcessName) {
    formatter::string_t formatter("{process:s}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_TRUE(writer.result().to_string().size() > 0);
}

TEST(string_t, Thread) {
    formatter::string_t formatter("{thread}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    std::ostringstream stream;
#ifdef __linux
    stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
    stream << std::this_thread::get_id();

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(string_t, ThreadExplicitly) {
    formatter::string_t formatter("{thread:#x}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    std::ostringstream stream;
#ifdef __linux
    stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
    stream << std::this_thread::get_id();

    EXPECT_EQ(stream.str(), writer.result().to_string());
}

TEST(string_t, ThreadExplicitlySpec) {
    formatter::string_t formatter("{thread:>#16x}");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

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
    formatter::string_t formatter("{thread:s}");

    threadname_guard guard("thread#0");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("thread#0", writer.result().to_string());
}

TEST(string_t, ThreadNameWithSpec) {
    formatter::string_t formatter("{thread:>10s}");

    threadname_guard guard("thread#0");

    const string_view message("-");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("  thread#0", writer.result().to_string());
}

TEST(string_t, Timestamp) {
    // NOTE: By default %Y-%m-%d %H:%M:%S.%f pattern is used.
    formatter::string_t formatter("[{timestamp}]");

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
    formatter.format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampExplicit) {
    formatter::string_t formatter("[{timestamp:{%Y}}]");

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
    formatter.format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampExplicitWithType) {
    formatter::string_t formatter("[{timestamp:{%H:%M:%S}s}]");

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
    formatter.format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampSpec) {
    formatter::string_t formatter("[{timestamp:>30s}]");

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
    formatter.format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, TimestampNum) {
    formatter::string_t formatter("[{timestamp:d}]");

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
    formatter.format(record, writer);

    EXPECT_EQ(wr.str(), writer.result().to_string());
}

TEST(string_t, Leftover) {
    formatter::string_t formatter("{...}");

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("key#1: 42, key#2: value#2"),
        Eq("key#2: value#2, key#1: 42")
    ));
}

TEST(string_t, LeftoverEmpty) {
    formatter::string_t formatter("[{...}]");

    const string_view message("-");
    const attribute_list attributes{};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[]", writer.result().to_string());
}

TEST(string_t, LeftoverWithPrefixAndSuffix) {
    formatter::string_t formatter("{...}");
    formatter.leftover("...", "[", "]", "{k}: {v}", ", ", false);

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("[key#1: 42, key#2: value#2]"),
        Eq("[key#2: value#2, key#1: 42]")
    ));
}

TEST(string_t, LeftoverEmptyWithPrefixAndSuffix) {
    formatter::string_t formatter("{...}");
    formatter.leftover("...", "[", "]", "{k}: {v}", ", ", false);

    const string_view message("-");
    const attribute_list attributes{};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("", writer.result().to_string());
}

TEST(DISABLED_string_t, LeftoverWithPattern) {
    formatter::string_t formatter("{...}");
    formatter.leftover("...", "", "", "{k}={v}", ", ", false);

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("key#1=42, key#2=value#2"),
        Eq("key#2=value#2, key#1=42")
    ));
}

TEST(string_t, LeftoverWithSeparator) {
    formatter::string_t formatter("{...}");
    formatter.leftover("...", "", "", "{k}={v}", " | ", false);

    const string_view message("-");
    const attribute_list attributes{{"key#1", {42}}, {"key#2", {"value#2"}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_THAT(writer.result().to_string(), AnyOf(
        Eq("key#1: 42 | key#2: value#2"),
        Eq("key#2: value#2 | key#1: 42")
    ));
}

TEST(string_t, Type) {
    EXPECT_EQ("string", std::string(factory<formatter::string_t>::type()));
}

}  // namespace testing
}  // namespace blackhole
