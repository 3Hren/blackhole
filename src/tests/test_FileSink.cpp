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

TEST(rotator_t, RotatingSequence) {
    sink::rotator::config_t config = { "test.log.%N", 1, 1024 };
    mock::files::backend_t backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    InSequence s;
    EXPECT_CALL(backend, flush());
    EXPECT_CALL(backend, close());
    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));
    EXPECT_CALL(backend, open());

    rotator.rotate();
}

TEST(rotator_t, RotateMultipleFiles) {
    sink::rotator::config_t config = { "test.log.%N", 2, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.1" })));
    EXPECT_CALL(backend, exists(_))
            .WillRepeatedly(Return(true));

    InSequence s;
    EXPECT_CALL(backend, rename("test.log.1", "test.log.2"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, NotRenameIfFileNotExists) {
    sink::rotator::config_t config = { "test.log.%N", 2, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log.1", "test.log.2"))
            .Times(0);
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, SubstitutesFilenamePlaceholder) {
    sink::rotator::config_t config = { "%(filename)s.%N", 1, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t> rotator(config, backend);

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

namespace {

std::time_t to_time_t(const std::string& message, const std::string& format = "%Y%m%d") {
    std::tm tm = {};
    strptime(message.c_str(), format.c_str(), &tm);
    std::time_t time = timegm(&tm);
    std::cout << time << " - " << std::put_time(std::gmtime(&time), format.c_str()) << std::endl;
    return time;
}

}

TEST(rotator_t, SubstitutesDateTimePlaceholders) {
    sink::rotator::config_t config = { "test.log.%Y%m%d", 1, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t, mock::timer_t> rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.20140127"));

    rotator.rotate();
}

TEST(rotator_t, RotateWithDateTimePlaceholders) {
    sink::rotator::config_t config = { "test.log.%Y%m%d", 2, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t, mock::timer_t> rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.20140126" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.20140127"));

    rotator.rotate();
}

TEST(rotator_t, RotateWithDateTimeAndCountPlaceholders) {
    sink::rotator::config_t config = { "test.log.%Y%m%d.%N", 2, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t, mock::timer_t> rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.20140126.1" })));
    EXPECT_CALL(backend, exists("test.log.20140126.1"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log.20140126.1", "test.log.20140126.2"));
    EXPECT_CALL(backend, rename("test.log", "test.log.20140127.1"));

    rotator.rotate();
}

TEST(rotator_t, RotateWithDateTimeAndCountPlaceholders2) {
    sink::rotator::config_t config = { "test.log.%Y%m%d.%N.wow!", 2, 1024 };
    NiceMock<mock::files::backend_t> backend("test.log");
    sink::rotator_t<mock::files::backend_t, mock::timer_t> rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.20140126.1.wow!" })));
    EXPECT_CALL(backend, exists("test.log.20140126.1.wow!"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log.20140126.1.wow!", "test.log.20140126.2.wow!"));
    EXPECT_CALL(backend, rename("test.log", "test.log.20140127.1.wow!"));

    rotator.rotate();
}
