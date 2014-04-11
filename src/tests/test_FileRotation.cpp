#include <blackhole/sink/files/rotation.hpp>

#include "global.hpp"
#include "mocks.hpp"
#include "mocks/files.hpp"

using namespace blackhole;

namespace mocked {

typedef sink::rotator_t<mock::files::backend_t, mock::files::rotation::watcher_t> rotator_t;

typedef sink::rotator_t<mock::files::backend_t, mock::files::rotation::watcher_t, mock::timer_t> rotator_with_timer_t;

}

namespace blackhole {
namespace sink {
namespace rotation {
namespace watcher {

template<>
struct config_t<mock::files::rotation::watcher_t> {};

} // namespace watcher
} // namespace rotation
} // namespace sink
} // namespace blackhole

TEST(rotator_t, Class) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N", 1 };
    mock::files::backend_t backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));
    mocked::rotator_t rotator(config, backend);
    UNUSED(rotator);
}

TEST(rotator_t, RotatingSequence) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N", 1 };
    mock::files::backend_t backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));
    mocked::rotator_t rotator(config, backend);

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
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N", 2 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.1" })));
    EXPECT_CALL(backend, exists(_))
            .WillRepeatedly(Return(true));

    InSequence s;
    EXPECT_CALL(backend, rename("test.log.1", "test.log.2"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, RotateWithOverridingBackups) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N", 2 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.1", "test.log.2" })));
    EXPECT_CALL(backend, exists(_))
            .WillRepeatedly(Return(true));
    EXPECT_CALL(backend, rename("test.log.1", "test.log.2"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, ProperRolloverWithAbsentFiles) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N", 10 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.9" })));
    EXPECT_CALL(backend, exists(_))
            .WillRepeatedly(Return(true));

    InSequence s;
    EXPECT_CALL(backend, rename("test.log.9", "test.log.2"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, IncreaseDigitsInTheMiddle) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N.123", 10 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.1.123" })));
    EXPECT_CALL(backend, exists(_))
            .WillRepeatedly(Return(true));

    InSequence s;
    EXPECT_CALL(backend, rename("test.log.1.123", "test.log.2.123"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1.123"));

    rotator.rotate();
}

TEST(rotator_t, NotRenameIfFileNotExists) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N", 2 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

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
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "%(filename)s.%N", 1 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, SubstitutesFilenamePlaceholderAndProperlyRotates) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "%(filename)s.%N", 2 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.1" })));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, exists("test.log.1"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log.1", "test.log.2"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1"));

    rotator.rotate();
}

TEST(rotator_t, SubstitutesLongFilenamePlaceholder) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "%(filename)s.%N", 2 };
    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillRepeatedly(Return("long_filename_test.log"));

    mocked::rotator_t rotator(config, backend);

    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "long_filename_test.log", "long_filename_test.log.1" })));
    EXPECT_CALL(backend, exists("long_filename_test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, exists("long_filename_test.log.1"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("long_filename_test.log.1", "long_filename_test.log.2"));
    EXPECT_CALL(backend, rename("long_filename_test.log", "long_filename_test.log.1"));

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
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%Y%m%d", 1 };
    NiceMock<mock::files::backend_t> backend;
    mocked::rotator_with_timer_t rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.20140127"));

    rotator.rotate();
}

TEST(rotator_t, RotateWithDateTimePlaceholders) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%Y%m%d", 2 };
    NiceMock<mock::files::backend_t> backend;
    mocked::rotator_with_timer_t rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log", "test.log.20140127"));

    rotator.rotate();
}

TEST(rotator_t, RotateWithDateTimeAndCountPlaceholders) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%Y%m%d.%N", 2 };
    NiceMock<mock::files::backend_t> backend;
    mocked::rotator_with_timer_t rotator(config, backend);

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

TEST(rotator_t, RotateWithDateTimePlaceholderBeforeCounter) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%Y%m%d.%N.wow!", 2 };
    NiceMock<mock::files::backend_t> backend;
    mocked::rotator_with_timer_t rotator(config, backend);

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

TEST(rotator_t, RotateWithDateTimePlaceholderAfterCounter) {
    sink::rotation::config_t<mock::files::rotation::watcher_t> config = { "test.log.%N.%Y%m%d.wow!", 2 };
    NiceMock<mock::files::backend_t> backend;
    mocked::rotator_with_timer_t rotator(config, backend);

    EXPECT_CALL(rotator.timer(), current())
            .Times(1)
            .WillOnce(Return(to_time_t("20140127")));

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, listdir())
            .WillOnce(Return(std::vector<std::string>({ "test.log", "test.log.1.20140126.wow!" })));
    EXPECT_CALL(backend, exists("test.log.1.20140126.wow!"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, exists("test.log"))
            .WillOnce(Return(true));
    EXPECT_CALL(backend, rename("test.log.1.20140126.wow!", "test.log.2.20140126.wow!"));
    EXPECT_CALL(backend, rename("test.log", "test.log.1.20140127.wow!"));

    rotator.rotate();
}

TEST(counter_t, ParserWithoutPlaceholders) {
    using namespace sink::rotation;
    EXPECT_EQ(counter_t({ "test.log", "", 0 }), counter_t::from_string("test.log"));
}

TEST(couter_t, ParseOnlyCounterPlaceholder) {
    using namespace sink::rotation;
    EXPECT_EQ(counter_t({ "test.log.", "", 1 }), counter_t::from_string("test.log.%N"));
    EXPECT_EQ(counter_t({ "test.log.", "", 1 }), counter_t::from_string("test.log.%1N"));
    EXPECT_EQ(counter_t({ "test.log.", "", 2 }), counter_t::from_string("test.log.%2N"));
    EXPECT_EQ(counter_t({ "test.log.", "", 9 }), counter_t::from_string("test.log.%9N"));
    EXPECT_EQ(counter_t({ "test.log.", "", 10 }), counter_t::from_string("test.log.%10N"));
    EXPECT_EQ(counter_t({ "test.log.", "", 100 }), counter_t::from_string("test.log.%100N"));
}

TEST(couter_t, ParseCounterPlaceholderWithDatetime) {
    using namespace sink::rotation;
    EXPECT_EQ(counter_t({ "test.log.", ".YYYYmmdd", 1 }), counter_t::from_string("test.log.%N.%Y%m%d"));
    EXPECT_EQ(counter_t({ "test.log.YYYYmmdd.", "", 1 }), counter_t::from_string("test.log.%Y%m%d.%N"));
}
