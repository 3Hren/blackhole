#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/handler.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/record.hpp>
#include <blackhole/root.hpp>

namespace blackhole {
namespace testing {

using ::testing::Invoke;
using ::testing::_;

namespace mock {
namespace {

class handler_t : public ::blackhole::handler_t {
public:
    MOCK_METHOD1(execute, void(const record_t&));
};

}  // namespace
}  // namespace mock

TEST(RootLogger, Log) {
    // Can be initialized with none handlers, does nothing.
    root_logger_t logger({});
    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, ConstLog) {
    const root_logger_t logger({});
    logger.log(0, "GET /porn.png HTTP/1.1");
}

TEST(RootLogger, DispatchRecordToHandlers) {
    std::vector<std::unique_ptr<handler_t>> handlers;
    std::vector<mock::handler_t*> handlers_view;

    for (int i = 0; i < 4; ++i) {
        std::unique_ptr<mock::handler_t> handler(new mock::handler_t);
        handlers_view.push_back(handler.get());
        handlers.push_back(std::move(handler));
    }

    const root_logger_t logger(std::move(handlers));

    for (auto handler : handlers_view) {
        EXPECT_CALL(*handler, execute(_))
            .Times(1)
            .WillOnce(Invoke([](const record_t& record) {
                EXPECT_EQ("GET /porn.png HTTP/1.1", record.message().to_string());
                EXPECT_EQ(0, record.severity());
                EXPECT_EQ(0, record.attributes().size());
            }));
    }

    logger.log(0, "GET /porn.png HTTP/1.1");
}

}  // namespace testing
}  // namespace blackhole
