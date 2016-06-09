#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/console.hpp>
#include <blackhole/termcolor.hpp>
#include <src/sink/console.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

TEST(builder, Default) {
    builder<console_t> builder;
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cout, &cast.stream());

    const string_view message("");
    const attribute_pack pack;

    record_t record(0, message, pack);
    EXPECT_EQ(termcolor_t(), cast.mapping(record));
}

TEST(builder, RedirectToOutputWithLvalue) {
    builder<console_t> builder;
    builder.stdout();
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cout, &cast.stream());
}

TEST(builder, RedirectToOutputWithRvalue) {
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

TEST(builder, RedirectToErrorWithRvalue) {
    auto sink = builder<console_t>()
        .stderr()
        .build();
    auto& cast = static_cast<console_t&>(*sink);

    EXPECT_EQ(&std::cerr, &cast.stream());
}

TEST(builder, ColorizeWithLvalue) {
    builder<console_t> builder;
    builder.colorize(0, termcolor_t::green());
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    const string_view message("");
    const attribute_pack pack;

    record_t record1(0, message, pack);
    EXPECT_EQ(termcolor_t::green(), cast.mapping(record1));

    record_t record2(1, message, pack);
    EXPECT_EQ(termcolor_t(), cast.mapping(record2));
}

TEST(builder, ColorizeWithRvalue) {
    auto sink = builder<console_t>()
        .colorize(0, termcolor_t::green())
        .build();
    auto& cast = static_cast<console_t&>(*sink);

    const string_view message("");
    const attribute_pack pack;

    record_t record1(0, message, pack);
    EXPECT_EQ(termcolor_t::green(), cast.mapping(record1));

    record_t record2(1, message, pack);
    EXPECT_EQ(termcolor_t(), cast.mapping(record2));
}

TEST(builder, ResetColorizeWithLvalue) {
    builder<console_t> builder;
    builder.colorize([](const record_t& record) -> termcolor_t {
        if (record.severity() == severity_t(0)) {
            return termcolor_t::green();
        } else {
            return termcolor_t::red();
        }
    });
    auto sink = std::move(builder).build();
    auto& cast = static_cast<console_t&>(*sink);

    const string_view message("");
    const attribute_pack pack;

    record_t record1(0, message, pack);
    EXPECT_EQ(termcolor_t::green(), cast.mapping(record1));

    record_t record2(1, message, pack);
    EXPECT_EQ(termcolor_t::red(), cast.mapping(record2));
}

TEST(builder, ResetColorizeWithRvalue) {
    auto sink = builder<console_t>()
        .colorize([](const record_t& record) -> termcolor_t {
            if (record.severity() == severity_t(0)) {
                return termcolor_t::green();
            } else {
                return termcolor_t::red();
            }
        })
        .build();
    auto& cast = static_cast<console_t&>(*sink);

    const string_view message("");
    const attribute_pack pack;

    record_t record1(0, message, pack);
    EXPECT_EQ(termcolor_t::green(), cast.mapping(record1));

    record_t record2(1, message, pack);
    EXPECT_EQ(termcolor_t::red(), cast.mapping(record2));
}

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
