#include "Mocks.hpp"

using namespace blackhole;

TEST(file_t, Class) {
    sink::file_t<> sink("test.log");
    UNUSED(sink);
}

TEST(file_t, WritesToTheFile) {
    sink::file_t<NiceMock<mock::files::backend_t>> sink("test.log");
    EXPECT_CALL(sink.backend(), write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message");
}

TEST(file_t, OpensFileIfItClosedWhenWriting) {
    sink::file_t<NiceMock<mock::files::backend_t>> sink("test.log");
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

TEST(file_t, ThrowsExceptionIfFileCannotBeOpened) {
    sink::file_t<NiceMock<mock::files::backend_t>> sink("test.log");
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

TEST(file_t, AutoFlushIfSpecified) {
    sink::file::config_t config("test.log", true);
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(1);

    sink.consume("message");
}

TEST(file_t, AutoFlushIsDisabledIfSpecified) {
    sink::file::config_t config("test.log", false);
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(0);

    sink.consume("message");
}

TEST(file_t, AutoFlushIsEnabledByDefault) {
    sink::file_t<NiceMock<mock::files::backend_t>> sink("test.log");
    EXPECT_CALL(sink.backend(), flush())
            .Times(1);
    sink.consume("message");
}
