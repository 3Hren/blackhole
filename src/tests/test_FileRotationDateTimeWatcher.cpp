#include "Mocks.hpp"

TEST(datetime_t, Class) {
    sink::rotation::watcher::datetime_t<> watcher;
    UNUSED(watcher);
}

TEST(datetime_t, EnumInitialization) {
    using namespace sink::rotation::watcher;

    datetime_t<> watcher1(config_t<datetime_t<>>::period_t::hourly);
    datetime_t<> watcher2(config_t<datetime_t<>>::period_t::daily);
    datetime_t<> watcher3(config_t<datetime_t<>>::period_t::weekly);
    datetime_t<> watcher4(config_t<datetime_t<>>::period_t::monthly);

    UNUSED(watcher1);
    UNUSED(watcher2);
    UNUSED(watcher3);
    UNUSED(watcher4);
}

namespace mock {
class time_picker_t {
public:
    time_picker_t() {
        std::tm timeinfo;
        std::memset(&timeinfo, 0, sizeof(timeinfo));
        ON_CALL(*this, now())
                .WillByDefault(Return(timeinfo));
    }

    MOCK_CONST_METHOD0(now, std::tm());
};
}

TEST(datetime_t, TriggerWhenDailyCounterIncreases) {
    using namespace sink::rotation::watcher;

    auto period = config_t<datetime_t<NiceMock<mock::time_picker_t>>>::period_t::daily;
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

    auto period = config_t<datetime_t<NiceMock<mock::time_picker_t>>>::period_t::daily;
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

    EXPECT_EQ(config_t<datetime_t<>>::period_t::hourly, datetime_t<>("H").period);
    EXPECT_EQ(config_t<datetime_t<>>::period_t::daily, datetime_t<>("d").period);
    EXPECT_EQ(config_t<datetime_t<>>::period_t::weekly, datetime_t<>("w").period);
    EXPECT_EQ(config_t<datetime_t<>>::period_t::monthly, datetime_t<>("M").period);
}
