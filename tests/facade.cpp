#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/extensions/facade.hpp>
#include <blackhole/scope/manager.hpp>

#include "mocks/logger.hpp"

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 50000

namespace std {

template<typename T>
auto operator==(const std::reference_wrapper<T>& lhs, const std::reference_wrapper<T>& rhs) -> bool {
    return lhs.get() == rhs.get();
}

}  // namespace std

#endif

namespace blackhole {
namespace testing {

using ::testing::An;
using ::testing::Invoke;
using ::testing::WithArg;
using ::testing::_;

typedef mock::logger_t logger_type;

TEST(Facade, Constructor) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    EXPECT_EQ(&inner, &logger.inner());
}

TEST(Facade, PrimitiveLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    EXPECT_CALL(inner, log(severity_t(0), string_view("GET /porn.png HTTP/1.0")))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.0");
}

TEST(Facade, AttributeLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    const attribute_list attributes{{"key#1", {42}}};
    attribute_pack expected{attributes};

    EXPECT_CALL(inner, log(severity_t(0), string_view("GET /porn.png HTTP/1.0"), expected))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.0", attribute_list{
        {"key#1", {42}}
    });
}

TEST(Facade, FormattedLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    attribute_pack expected;
    writer_t writer;

    EXPECT_CALL(inner, log(severity_t(0), An<const lazy_message_t&>(), expected))
        .Times(1)
        .WillOnce(WithArg<1>(Invoke([](const lazy_message_t& message) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - {}", message.pattern.to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 42", message.supplier().to_string());
        })));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", 42);
}

TEST(Facade, LazyFormattedLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    attribute_pack expected;
    writer_t writer;

    EXPECT_CALL(inner, log(severity_t(0), An<const lazy_message_t&>(), expected))
        .Times(1)
        .WillOnce(WithArg<1>(Invoke([](const lazy_message_t& message) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - {}", message.pattern.to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 42", message.supplier().to_string());
        })));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", [](std::ostream& stream) -> std::ostream& {
        return stream << 42;
    });
}

TEST(Facade, LazyFormattedLogCaptured) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    attribute_pack expected;
    writer_t writer;

    EXPECT_CALL(inner, log(severity_t(0), An<const lazy_message_t&>(), expected))
        .Times(1)
        .WillOnce(WithArg<1>(Invoke([](const lazy_message_t& message) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - {}", message.pattern.to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 42", message.supplier().to_string());
        })));

    const auto captured = 42;
    logger.log(0, "GET /porn.png HTTP/1.0 - {}", [&](std::ostream& stream) -> std::ostream& {
        return stream << captured;
    });
}

namespace {

struct int_wrapper_t {
    int v;
};

auto operator<<(std::ostream& stream, const int_wrapper_t& v) -> std::ostream& {
    return stream << v.v;
}

} // namespace

TEST(Facade, LazyFormattedLogStream) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    attribute_pack expected;
    writer_t writer;

    EXPECT_CALL(inner, log(severity_t(0), An<const lazy_message_t&>(), expected))
        .Times(1)
        .WillOnce(WithArg<1>(Invoke([](const lazy_message_t& message) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - {}", message.pattern.to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 42", message.supplier().to_string());
        })));


    logger.log(0, "GET /porn.png HTTP/1.0 - {}", int_wrapper_t{42});
}

TEST(Facade, FormattedAttributeLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    const attribute_list attributes{{"key#1", {42}}};
    attribute_pack expected{attributes};

    EXPECT_CALL(inner, log(severity_t(0), An<const lazy_message_t&>(), expected))
        .Times(1)
        .WillOnce(WithArg<1>(Invoke([](const lazy_message_t& message) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - {}", message.pattern.to_string());
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 2345", message.supplier().to_string());
        })));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", 2345, attribute_list{
        {"key#1", {42}}
    });
}

}  // namespace testing
}  // namespace blackhole
