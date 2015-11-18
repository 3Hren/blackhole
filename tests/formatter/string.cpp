#include <gtest/gtest.h>

#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
namespace testing {

TEST(string_t, MessagePlaceholder) {
    formatter::string_t formatter("[{message}]");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[value]", writer.result().to_string());
}

TEST(string_t, NumericSeverityWithMessagePlaceholders) {
    formatter::string_t formatter("[{severity:d}]: {message}");

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ("[0]: value", writer.result().to_string());
}

// TODO: Check error when setting an option to reserved name, i.e. timestamp or message.
// TODO: Check error when ph in pattern was not found.

}  // namespace testing
}  // namespace blackhole
