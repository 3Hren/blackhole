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

TEST(builder, RedirectToOutputWithLvalue) {
    builder<console_t> builder;
    builder.stdout();
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cout, &cast.stream());
}

TEST(builder, RedirectToOutputWithPRvalue) {
    auto sink = builder<console_t>()
        .stdout()
        .build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cout, &cast.stream());
}

TEST(builder, RedirectToErrorWithLvalue) {
    builder<console_t> builder;
    builder.stderr();
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cerr, &cast.stream());
}

TEST(builder, RedirectToErrorWithPRvalue) {
    auto sink = builder<console_t>()
        .stderr()
        .build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cerr, &cast.stream());
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
