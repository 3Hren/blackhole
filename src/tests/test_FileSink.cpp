#include <blackhole/sink/files.hpp>

#include "global.hpp"
#include "mocks/files.hpp"

using namespace blackhole;

TEST(files_t, Class) {
    sink::files_t<>::config_type config("test.log");
    sink::files_t<> sink(config);
    UNUSED(sink);
}

TEST(files_t, WritesToTheFile) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message");
}

TEST(files_t, OpensFileIfItClosedWhenWriting) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), opened()).
            Times(1).
            WillOnce(Return(false));
    EXPECT_CALL(sink.backend(), open()).
            Times(1).
            WillOnce(Return(true));
    EXPECT_CALL(sink.backend(), write(_)).
            Times(1);
    sink.consume("message");
}

TEST(files_t, ThrowsExceptionIfFileCannotBeOpened) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), opened())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(sink.backend(), open())
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(sink.backend(), write(_))
            .Times(0);
    EXPECT_THROW(sink.consume("message"), blackhole::error_t);
}

TEST(files_t, AutoFlushIfSpecified) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log", true);
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(1);

    sink.consume("message");
}

TEST(files_t, AutoFlushIsDisabledIfSpecified) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log", false);
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(0);

    sink.consume("message");
}

TEST(files_t, AutoFlushIsEnabledByDefault) {
    sink::files_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::files_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(1);
    sink.consume("message");
}
