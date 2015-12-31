#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/attribute.hpp>
#include <blackhole/extensions/facade.hpp>

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

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0")))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.0");
}

TEST(Facade, AttributeLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    const attribute_list attributes{{"key#1", {42}}};
    attribute_pack expected{attributes};

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0"), expected))
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

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0 - {}"), expected, _))
        .Times(1)
        .WillOnce(WithArg<3>(Invoke([](logger_type::supplier_t fn) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 42", fn().to_string());
        })));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", 42);
}

TEST(Facade, FormattedAttributeLog) {
    logger_type inner;
    logger_facade<logger_type> logger(inner);

    const attribute_list attributes{{"key#1", {42}}};
    attribute_pack expected{attributes};

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0 - {}"), expected, _))
        .Times(1)
        .WillOnce(WithArg<3>(Invoke([](logger_type::supplier_t fn) {
            EXPECT_EQ("GET /porn.png HTTP/1.0 - 2345", fn().to_string());
        })));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", 2345, attribute_list{
        {"key#1", {42}}
    });
}

}  // namespace testing
}  // namespace blackhole
