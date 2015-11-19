#include <gtest/gtest.h>

#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
namespace testing {

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
    formatter::string_t formatter("{protocol}{version:.1f}", {
        {"version", formatter::option::optional_t{"/", " - REQUIRED"}}
    });

    const string_view message("-");
    const attribute_list attributes{{"protocol", {"HTTP"}}, {"version", {1.1}}};
    const attribute_pack pack{attributes};
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("HTTP/1.1 - REQUIRED", writer.result().to_string());
}

TEST(string_t, ThrowsIfOptionsContainsReservedPlaceholderNames) {
    using formatter::option::optional_t;

    EXPECT_THROW((formatter::string_t("{protocol}", {{"message", optional_t{"[", "]"}}})),
        std::logic_error);
}

}  // namespace testing
}  // namespace blackhole
