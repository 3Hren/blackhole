#pragma once

#include "keyword/process.hpp"
#include "universe.hpp"

namespace blackhole {

namespace aux {

//! Universe attributes initializer.
class bigbang_t {
public:
    bigbang_t() {
        auto& storage = universe_storage_t::instance();
        storage.add(keyword::pid() = getpid());
    }
};

} // namespace aux

} // namespace blackhole
