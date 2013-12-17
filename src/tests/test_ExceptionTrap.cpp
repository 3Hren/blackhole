#include "Mocks.hpp"

namespace testing {

struct handler_t {
    void operator()() {}
};

} // namespace testing

TEST(exception_trap_t, DoNotCrashWhenRaisedExceptionFromFrontend) {
    auto frontend = std::make_unique<NiceMock<mock::frontend_t>>();
    EXPECT_CALL(*frontend.get(), handle(_))
            .Times(1)
            .WillOnce(Throw(std::runtime_error("error")));

    logger_base_t log;
    log.add_frontend(std::move(frontend));
    log.set_exception_handler(testing::handler_t());
    log::record_t record;
    log.push(std::move(record));
}
