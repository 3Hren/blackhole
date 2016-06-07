#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/sink/console.hpp>
#include <src/sink/console.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using experimental::builder;

TEST(builder, Default) {
    builder<console_t> builder;
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cout, &cast.stream());
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
