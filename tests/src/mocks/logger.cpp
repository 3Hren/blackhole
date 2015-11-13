#include "mocks/logger.hpp"

#include <blackhole/detail/unimplemented.hpp>

namespace blackhole {
namespace testing {
namespace mock {

logger_t::logger_t() {}

logger_t::~logger_t() {}

auto logger_t::scoped(attributes_t) -> scoped_t {
    BLACKHOLE_UNIMPLEMENTED();
    std::abort();
}

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
