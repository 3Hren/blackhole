#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/extensions/format.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/record.hpp>
#include <blackhole/root.hpp>

namespace blackhole {
namespace testing {

using ::testing::_;

namespace mock {
namespace {

class handler_t : public ::blackhole::handler_t {
public:
    MOCK_METHOD1(execute, void(const record_t&));
};

}  // namespace
}  // namespace mock

TEST(Root, Log) {
    // Can be initialized with none handlers, does nothing.
    root_logger_t logger({});
    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(Root, ForwardToHandler) {
    std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
    auto& mock = *handler.get();

    std::vector<std::unique_ptr<handler_t>> handlers;
    handlers.push_back(std::move(handler));

    root_logger_t logger(std::move(handlers));

    EXPECT_CALL(mock, execute(_))
        .Times(1);

    logger.log(0, "GET /porn.png HTTP/1.1");
}

// TEST(wrapper, call) {
//     using attribute::value_t;
//     using attribute::owned_t;
//
//     root_logger_t root({});
//
//     wrapper_t wrapper1{root, {
//         {"key#0", owned_t(0)},
//         {"key#1", owned_t("value#1")}
//     }};
//
//     wrapper_t wrapper2{wrapper1, {
//         {"key#2", owned_t(2)},
//         {"key#3", owned_t("value#3")}
//     }};
//
//     logger_facade<wrapper_t> logger(wrapper2);
//
//     logger.log(0,
//         {
//             {"key#4", value_t(42)},
//             {"key#5", value_t(3.1415)},
//             {"key#6", value_t("value")}
//         }, "{} - {} [{}] 'GET {} HTTP/1.0' {} {}",
//         "[::]",
//         "esafronov",
//         "10/Oct/2000:13:55:36 -0700",
//         "/porn.png",
//         200,
//         2326
//     );
// }

}  // namespace testing
}  // namespace blackhole
