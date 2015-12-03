#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attributes.hpp>
#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter/json.hpp>
#include <blackhole/record.hpp>

namespace blackhole {
namespace testing {
namespace formatter {

using ::blackhole::formatter::json_t;

TEST(json_t, FormatMessage) {
    json_t formatter;

    const string_view message("value");
    const attribute_pack pack;
    record_t record(0, message, pack);
    writer_t writer;
    formatter.format(record, writer);

    EXPECT_EQ(R"({"message":"value"})", writer.result().to_string());
}

}  // namespace formatter
}  // namespace testing
}  // namespace blackhole
