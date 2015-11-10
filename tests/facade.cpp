#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/extensions/facade.hpp>
#include <blackhole/logger.hpp>

namespace blackhole {
namespace testing {

using ::testing::_;

namespace mock {
namespace {

class logger_t : public ::blackhole::logger_t {
public:
    MOCK_CONST_METHOD2(log, void(int, string_view));
    MOCK_CONST_METHOD3(log, void(int, string_view, range_t&));
    MOCK_CONST_METHOD4(log, void(int, string_view, range_t&, const format_t&));
};

}  // namespace
}  // namespace mock

TEST(Facade, PrimitiveLog) {
    typedef mock::logger_t logger_type;

    const logger_type inner{};
    logger_facade<logger_type> logger(inner);

    EXPECT_CALL(inner, log(0, string_view("GET /porn.png HTTP/1.0")))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.0");
}

}  // namespace testing
}  // namespace blackhole
