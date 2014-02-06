#include "Mocks.hpp"

TEST(datetime_t, Class) {
    sink::rotation::watcher::datetime_t watcher;
    UNUSED(watcher);
}

TEST(datetime_t, EnumInitialization) {
    using namespace sink::rotation::watcher;
    datetime_t watcher;
    watcher = datetime_t(config_t<datetime_t>::period_t::hourly);
    watcher = datetime_t(config_t<datetime_t>::period_t::daily);
    watcher = datetime_t(config_t<datetime_t>::period_t::weekly);
    watcher = datetime_t(config_t<datetime_t>::period_t::monthly);
}
