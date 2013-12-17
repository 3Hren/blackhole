#include "Mocks.hpp"

namespace mock {
class exception_handler_t : public log::exception_handler_t {
public:
    MOCK_CONST_METHOD0(execute, void());
};
}

TEST(exception_trap_t, CatchingExceptionsFromFrontend) {
    auto frontend = std::make_unique<NiceMock<mock::frontend_t>>();
    EXPECT_CALL(*frontend.get(), handle(_))
            .Times(1)
            .WillOnce(Throw(std::runtime_error("error")));

    auto handler = std::make_unique<mock::exception_handler_t>();
    EXPECT_CALL(*handler.get(), execute())
            .Times(1);

    logger_base_t log;
    log.add_frontend(std::move(frontend));
    log.set_exception_handler(std::move(handler));
    log::record_t record;
    log.push(std::move(record));
}
