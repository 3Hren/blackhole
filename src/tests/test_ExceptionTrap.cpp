#include <blackhole/attribute.hpp>
#include <blackhole/error/handler.hpp>
#include <blackhole/logger.hpp>

#include "global.hpp"
#include "mocks/frontend.hpp"

using namespace blackhole;

namespace testing {

struct simple_handler_t {
    void operator()() {}
};

} // namespace testing

TEST(exception_trap_t, SimpleExceptionHandler) {
    auto frontend = utils::make_unique<NiceMock<mock::frontend_t>>();
    EXPECT_CALL(*frontend.get(), handle(_))
            .Times(1)
            .WillOnce(Throw(std::runtime_error("error")));

    logger_base_t log;
    log.add_frontend(std::move(frontend));
    log.set_exception_handler(testing::simple_handler_t());
    log::record_t record;
    log.push(std::move(record));
}

namespace testing {

struct typed_handler_t {
    void operator()(const std::runtime_error&) {}
};

} // namespace testing

TEST(exception_trap_t, TypedExceptionHandler) {
    auto frontend = utils::make_unique<NiceMock<mock::frontend_t>>();
    EXPECT_CALL(*frontend.get(), handle(_))
            .Times(1)
            .WillOnce(Throw(std::runtime_error("error")));

    auto handler = log::exception::handler_factory_t<testing::typed_handler_t>::make<
        std::runtime_error
    >();

    logger_base_t log;
    log.add_frontend(std::move(frontend));
    log.set_exception_handler(handler);
    log::record_t record;
    log.push(std::move(record));
}

namespace testing {

struct sequenced_typed_handler_t {
    void operator()(const std::runtime_error&) {}

    void operator()(const std::exception&) {}
};

} // namespace testing

TEST(exception_trap_t, StrongSequencedTypedExceptionHandler) {
    auto frontend = utils::make_unique<NiceMock<mock::frontend_t>>();
    EXPECT_CALL(*frontend.get(), handle(_))
            .Times(1)
            .WillOnce(Throw(std::runtime_error("error")));

    auto handler = log::exception::handler_factory_t<testing::sequenced_typed_handler_t>::make<
        std::runtime_error,
        std::exception
    >();

    logger_base_t log;
    log.add_frontend(std::move(frontend));
    log.set_exception_handler(handler);
    log::record_t record;
    log.push(std::move(record));
}

