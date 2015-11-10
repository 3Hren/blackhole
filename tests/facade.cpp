#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/extensions/facade.hpp>
#include <blackhole/logger.hpp>

#include "mocks/logger.hpp"

namespace blackhole {
namespace testing {

using ::testing::ByRef;
using ::testing::InvokeArgument;
using ::testing::_;

typedef mock::logger_t logger_type;

TEST(Facade, PrimitiveLog) {
    const logger_type inner{};
    const logger_facade<logger_type> logger(inner);

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0")))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.0");
}

TEST(Facade, AttributeLog) {
    const logger_type inner{};
    const logger_facade<logger_type> logger(inner);

    const attribute_list attributes{{"key#1", {42}}};
    range_t expected{attributes};

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0"), expected))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.0", attribute_list{
        {"key#1", {42}}
    });
}

TEST(Facade, FormattedLog) {
    const logger_type inner{};
    const logger_facade<logger_type> logger(inner);

    range_t expected;
    writer_t writer;

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0 - {}"), expected, _))
        .Times(1)
        .WillOnce(InvokeArgument<3>(ByRef(writer)));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", 42);

    EXPECT_EQ("GET /porn.png HTTP/1.0 - 42", writer.inner.str());
}

TEST(Facade, FormattedAttributeLog) {
    const logger_type inner{};
    const logger_facade<logger_type> logger(inner);

    const attribute_list attributes{{"key#1", {42}}};
    range_t expected{attributes};
    writer_t writer;

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0 - {}"), expected, _))
        .Times(1)
        .WillOnce(InvokeArgument<3>(ByRef(writer)));

    logger.log(0, "GET /porn.png HTTP/1.0 - {}", 2345, attribute_list{
        {"key#1", {42}}
    });

    EXPECT_EQ("GET /porn.png HTTP/1.0 - 2345", writer.inner.str());
}

}  // namespace testing
}  // namespace blackhole
