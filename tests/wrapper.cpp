#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/logger.hpp>
#include <blackhole/wrapper.hpp>

#include <blackhole/root.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/extensions/format.hpp>

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

TEST(Wrapper, Constructor) {
    using attribute::value_t;
    using attribute::view_t;

    mock::logger_t logger;

    wrapper_t wrapper(logger, {
        {"key#0", value_t(0)},
        {"key#1", value_t("value#1")}
    });

    const view_of<attributes_t>::type expected = {
        {"key#0", view_t(0)},
        {"key#1", view_t("value#1")}
    };

    EXPECT_EQ(expected, wrapper.attributes());
}

// p   ::= p f{}
// p f ::= p f a{}
// p a
// p f a
TEST(__TESTING__, __API__) {
    using attribute::view_t;

    root_logger_t root({});
    logger_facade<root_logger_t> logger(root);

    logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
        "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326
    );

    logger.log(0, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
        "[::]", "esafronov", "10/Oct/2000:13:55:36 -0700", "/porn.png", 200, 2326,
        attribute_list{
            {"key#6", view_t(42)},
            {"key#7", view_t(3.1415)},
            {"key#8", view_t("value")}
        }
    );
}

}  // namespace testing
}  // namespace blackhole
