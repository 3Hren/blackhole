#include "mocks/logger.hpp"

#include <blackhole/detail/unimplemented.hpp>

namespace blackhole {
namespace testing {
namespace mock {

logger_t::logger_t() {}

logger_t::~logger_t() {}

auto logger_t::scoped(attributes_t attributes) -> scoped_t {
    return _scoped(std::move(attributes))();
}

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
