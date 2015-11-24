#include <gtest/gtest.h>

#include <blackhole/record.hpp>
#include <blackhole/sink/null.hpp>

namespace blackhole {
namespace testing {

using sink::null_t;

TEST(null_t, FilterOut) {
    const string_view message("-");
    const attribute_pack pack;
    record_t record(42, message, pack);

    null_t sink;

    EXPECT_FALSE(sink.filter(record));
}

}  // namespace testing
}  // namespace blackhole
