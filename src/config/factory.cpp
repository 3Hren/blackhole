#include "blackhole/config/factory.hpp"

#include "blackhole/forward.hpp"

#include "../util/deleter.hpp"

namespace blackhole {
inline namespace v1 {
namespace config {

factory_t::~factory_t() = default;

} // namespace config

template auto deleter_t::operator()(config::factory_t* value) -> void;

} // namespace v1
} // namespace blackhole
