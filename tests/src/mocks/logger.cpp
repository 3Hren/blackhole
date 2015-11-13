#include "mocks/logger.hpp"

namespace blackhole {
namespace testing {
namespace mock {

logger_t::logger_t() {}

logger_t::~logger_t() {}

auto logger_t::scoped(attributes_t) -> scoped_t {
    return scoped_t();
}

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
