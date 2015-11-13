#include "blackhole/logger.hpp"

#include "blackhole/scoped.hpp"

#include "blackhole/detail/unimplemented.hpp"

namespace blackhole {

logger_t::~logger_t() {}

auto
logger_t::scoped(attributes_t attributes) -> scoped_t {
    BLACKHOLE_UNIMPLEMENTED();
    return scoped_t();
}

}  // namespace blackhole
