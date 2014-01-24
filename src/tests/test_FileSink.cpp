#include "Mocks.hpp"

using namespace blackhole;

TEST(file_t, Class) {
    sink::file_t<>::config_type config("test.log");
    sink::file_t<> sink(config);
    UNUSED(sink);
}

TEST(file_t, WritesToTheFile) {
    sink::file_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), write(std::string("formatted message"))).
            Times(1);
    sink.consume("formatted message");
}

TEST(file_t, OpensFileIfItClosedWhenWriting) {
    sink::file_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
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
    sink::file_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
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
    sink::file_t<NiceMock<mock::files::backend_t>>::config_type config("test.log", true);
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(1);

    sink.consume("message");
}

TEST(file_t, AutoFlushIsDisabledIfSpecified) {
    sink::file_t<NiceMock<mock::files::backend_t>>::config_type config("test.log", false);
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(0);

    sink.consume("message");
}

TEST(file_t, AutoFlushIsEnabledByDefault) {
    sink::file_t<NiceMock<mock::files::backend_t>>::config_type config("test.log");
    sink::file_t<NiceMock<mock::files::backend_t>> sink(config);
    EXPECT_CALL(sink.backend(), flush())
            .Times(1);
    sink.consume("message");
}

//!@todo:
//! Given: size=1024, backup=1, suffix='.%N'
//! Condition: `backend.size()`=1025
//! Action: `rotator.rotate()`.

TEST(rotator_t, Class) {
    mock::files::backend_t backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(backend);
    UNUSED(rotator);
}

TEST(rotator_t, RotateFiles) {
    sink::rotator::config_t config = { 1024, 1, ".%N" };
    mock::files::backend_t backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    EXPECT_CALL(backend, flush())
            .Times(1);
    EXPECT_CALL(backend, close())
            .Times(1);
    EXPECT_CALL(backend, filename())
            .Times(1)
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, exists("test.log"))
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"))
            .Times(1);
    EXPECT_CALL(backend, open())
            .Times(1);
    rotator.rotate();
}

TEST(rotator_t, RotateMultipleFiles) {
    sink::rotator::config_t config = { 1024, 2, ".%N" };
    mock::files::backend_t backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    EXPECT_CALL(backend, flush())
            .Times(1);
    EXPECT_CALL(backend, close())
            .Times(1);
    EXPECT_CALL(backend, filename())
            .Times(1)
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, exists(_))
            .WillRepeatedly(Return(true));
    EXPECT_CALL(backend, rename("test.log.1", "test.log.2"))
            .Times(1);
    EXPECT_CALL(backend, rename("test.log", "test.log.1"))
            .Times(1);
    EXPECT_CALL(backend, open())
            .Times(1);
    rotator.rotate();
}

TEST(rotator_t, NotRenameIfFileNotExists) {
    sink::rotator::config_t config = { 1024, 2, ".%N" };
    mock::files::backend_t backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    EXPECT_CALL(backend, flush())
            .Times(1);
    EXPECT_CALL(backend, close())
            .Times(1);
    EXPECT_CALL(backend, filename())
            .Times(1)
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, exists("test.log.1"))
            .Times(1)
            .WillOnce(Return(false));
    EXPECT_CALL(backend, exists("test.log"))
            .Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"))
            .Times(1);
    EXPECT_CALL(backend, open())
            .Times(1);
    rotator.rotate();
}
