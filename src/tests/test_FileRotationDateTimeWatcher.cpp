#include <blackhole/sink/files/rotation/watcher/datetime.hpp>

#include "global.hpp"
#include "mocks.hpp"
#include "mocks/files.hpp"

using namespace blackhole;

TEST(datetime_t, Class) {
    sink::rotation::watcher::datetime_t<> watcher;
    UNUSED(watcher);
}

TEST(datetime_t, EnumInitialization) {
    using namespace sink::rotation::watcher;

    datetime_t<> watcher1(datetime::period_t::hourly);
    datetime_t<> watcher2(datetime::period_t::daily);
    datetime_t<> watcher3(datetime::period_t::weekly);
    datetime_t<> watcher4(datetime::period_t::monthly);

    UNUSED(watcher1);
    UNUSED(watcher2);
    UNUSED(watcher3);
    UNUSED(watcher4);
}

TEST(datetime_t, TriggerWhenDailyCounterIncreases) {
    using namespace sink::rotation::watcher;

    auto period = datetime::period_t::daily;
    datetime_t<NiceMock<mock::time_picker_t>> watcher(period);

    NiceMock<mock::files::backend_t> backend;

    std::tm timeinfo;
    std::memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_mday = 1;

    EXPECT_CALL(watcher.picker, now())
            .Times(1)
            .WillOnce(Return(timeinfo));

    EXPECT_TRUE(watcher(backend, "message"));
}

TEST(datetime_t, NotTriggerIfDailyCounterTheSame) {
    using namespace sink::rotation::watcher;

    auto period = datetime::period_t::daily;
    datetime_t<NiceMock<mock::time_picker_t>> watcher(period);

    NiceMock<mock::files::backend_t> backend;

    std::tm timeinfo;
    std::memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_mday = 0;

    EXPECT_CALL(watcher.picker, now())
            .Times(1)
            .WillOnce(Return(timeinfo));

    EXPECT_FALSE(watcher(backend, "message"));
}

TEST(datetime_t, InitializationFromString) {
    using namespace sink::rotation::watcher;

    EXPECT_EQ(datetime::period_t::hourly, datetime_t<>("H").period);
    EXPECT_EQ(datetime::period_t::daily, datetime_t<>("d").period);
    EXPECT_EQ(datetime::period_t::weekly, datetime_t<>("w").period);
    EXPECT_EQ(datetime::period_t::monthly, datetime_t<>("M").period);
}

TEST(watcher_set, TriggerIfDailyCounterIncreasesButSizeNot) {
    using namespace sink::rotation;
    typedef watcher::watcher_set<
        watcher::size_t,
        watcher::datetime_t<NiceMock<mock::time_picker_t>>
    > watcher_type;

    watcher::config_t<watcher_type> config;
    config.size = 1024;
    config.period = "d";

    watcher_type w(config);

    NiceMock<mock::files::backend_t> backend;
    std::tm timeinfo;
    std::memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_mday = 1;

    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size() - 1));
    EXPECT_CALL(w.picker, now())
            .Times(1)
            .WillOnce(Return(timeinfo));

    EXPECT_TRUE(w(backend, "message"));
}

TEST(watcher_set, TriggerIfDailyCounterNotIncreasesButSizeIs) {
    using namespace sink::rotation;
    typedef watcher::watcher_set<
        watcher::size_t,
        watcher::datetime_t<NiceMock<mock::time_picker_t>>
    > watcher_type;

    watcher::config_t<watcher_type> config;
    config.size = 1024;
    config.period = "d";

    watcher_type w(config);

    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size()));
    // Datetime watcher won't be called, because size watcher succeed.
    EXPECT_CALL(w.picker, now())
            .Times(0);

    EXPECT_TRUE(w(backend, "message"));
}

TEST(watcher_set, NotTriggerIfBothWatchersNotTriggers) {
    using namespace sink::rotation;
    typedef watcher::watcher_set<
        watcher::size_t,
        watcher::datetime_t<NiceMock<mock::time_picker_t>>
    > watcher_type;

    watcher::config_t<watcher_type> config;
    config.size = 1024;
    config.period = "d";

    watcher_type w(config);

    NiceMock<mock::files::backend_t> backend;
    EXPECT_CALL(backend, filename())
            .WillOnce(Return("test.log"));
    EXPECT_CALL(backend, size(std::string("test.log")))
            .WillOnce(Return(1024 - std::string("message").size() - 1));
    EXPECT_CALL(w.picker, now())
            .Times(1);

    EXPECT_FALSE(w(backend, "message"));
}

