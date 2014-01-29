#include "Mocks.hpp"

using namespace blackhole;

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
    sink::rotation::config_t config = { "test.log.%N", 1, 1024 };
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
    sink::rotation::config_t config = { "test.log.%N", 2, 1024 };
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
    sink::rotation::config_t config = { "test.log.%N", 2, 1024 };
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
    sink::rotation::config_t config = { "%(filename)s.%N", 1, 1024 };
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
    std::tm tm;
    std::memset(&tm, 0, sizeof(tm));
    strptime(message.c_str(), format.c_str(), &tm);
    std::time_t time = timegm(&tm);
    return time;
}

}

TEST(rotator_t, SubstitutesDateTimePlaceholders) {
    sink::rotation::config_t config = { "test.log.%Y%m%d", 1, 1024 };
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
    sink::rotation::config_t config = { "test.log.%Y%m%d", 2, 1024 };
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
    sink::rotation::config_t config = { "test.log.%Y%m%d.%N", 2, 1024 };
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

TEST(rotator_t, RotateWithDateTimeAndCountPlaceholdersInTheMiddleOfPattern) {
    sink::rotation::config_t config = { "test.log.%Y%m%d.%N.wow!", 2, 1024 };
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
