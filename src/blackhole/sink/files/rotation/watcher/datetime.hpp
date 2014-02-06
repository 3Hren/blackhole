#pragma once

#include "blackhole/sink/files/rotation/watcher/config.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

struct datetime_t {
    datetime_t(config_t<datetime_t>::period_t period = config_t<datetime_t>::period_t::daily) {

    }
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
