#include "Mocks.hpp"

namespace testing {

struct handler_t {
    void operator()(const std::runtime_error&) {}
};

} // namespace testing

TEST(exception_trap_t, DoNotCrashWhenRaisedExceptionFromFrontend) {
    auto frontend = std::make_unique<NiceMock<mock::frontend_t>>();
    EXPECT_CALL(*frontend.get(), handle(_))
            .Times(1)
            .WillOnce(Throw(std::runtime_error("error")));

    auto handler = log::exception::handler_factory_t<testing::handler_t>::make<
        std::runtime_error
    >();

    logger_base_t log;
    log.add_frontend(std::move(frontend));
    log.set_exception_handler(handler);
    log::record_t record;
    log.push(std::move(record));
}
