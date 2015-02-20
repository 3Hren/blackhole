#pragma once

#include <string>

#include "blackhole/config.hpp"

#include "blackhole/sink/files/rotation/watcher/config.hpp"

BLACKHOLE_BEG_NS

namespace sink {

namespace rotation {

namespace watcher {

struct move_t {
    static const char* name() {
        return "move";
    }

    move_t(const config_t<move_t>&) {}

    template<typename Backend>
    bool operator()(Backend& backend, const std::string&) const {
        return !backend.exists(backend.filename());
    }
};

} // namespace rotation

} // namespace watcher

} // namespace sink

BLACKHOLE_END_NS
