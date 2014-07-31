#include <blackhole/sink/files.hpp>

#include "global.hpp"
#include "mocks/files.hpp"

using namespace blackhole;

TEST(files_t, Class) {
    sink::files_t<>::config_type config("test.log");
    sink::files_t<> sink(config);
    UNUSED(sink);
}

TEST(files_t, DefaultBackendIsThreadUnsafe) {
    static_assert(
        sink::thread_safety<
            sink::files_t<>
        >::type::value == sink::thread::safety_t::unsafe,
        "`files_t<>` sink must be thread safe"
    );
}

TEST(files_t, WritesToTheFile) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    //!@note: It is needed for first file handler creation.
    sink.consume("", {});
    const auto& handlers = sink.handlers();
    auto it = handlers.find("test.log");
    ASSERT_TRUE(it != handlers.end());
    const auto& backend = it->second->backend();
    EXPECT_CALL(backend, write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message", {});
}

TEST(files_t, OpensFileIfItClosedWhenWriting) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);

    sink.consume("", {});
    const auto& handlers = sink.handlers();
    auto it = handlers.find("test.log");
    ASSERT_TRUE(it != handlers.end());
    const auto& backend = it->second->backend();

    EXPECT_CALL(backend, opened()).
            Times(1).
            WillOnce(Return(false));
    EXPECT_CALL(backend, open()).
            Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(backend, write(_)).
            Times(1);
    sink.consume("message", {});
}

TEST(files_t, ThrowsExceptionIfFileCannotBeOpened) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);

    sink.consume("", {});
    const auto& handlers = sink.handlers();
    auto it = handlers.find("test.log");
    ASSERT_TRUE(it != handlers.end());
    const auto& backend = it->second->backend();

    EXPECT_CALL(backend, opened())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(backend, open())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(backend, write(_))
            .Times(0);
    EXPECT_THROW(sink.consume("message", {}), blackhole::error_t);
}

TEST(files_t, AutoFlushIfSpecified) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log", true);
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);

    sink.consume("", {});
    const auto& handlers = sink.handlers();
    auto it = handlers.find("test.log");
    ASSERT_TRUE(it != handlers.end());
    const auto& backend = it->second->backend();

    EXPECT_CALL(backend, flush())
            .Times(1);

    sink.consume("message", {});
}

TEST(files_t, AutoFlushIsDisabledIfSpecified) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log", false);
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);

    sink.consume("", {});
    const auto& handlers = sink.handlers();
    auto it = handlers.find("test.log");
    ASSERT_TRUE(it != handlers.end());
    const auto& backend = it->second->backend();

    EXPECT_CALL(backend, flush())
            .Times(0);

    sink.consume("message", {});
}

TEST(files_t, AutoFlushIsEnabledByDefault) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);

    sink.consume("", {});
    const auto& handlers = sink.handlers();
    auto it = handlers.find("test.log");
    ASSERT_TRUE(it != handlers.end());
    const auto& backend = it->second->backend();

    EXPECT_CALL(backend, flush())
            .Times(1);
    sink.consume("message", {});
}
