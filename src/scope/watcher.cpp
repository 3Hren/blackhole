#include "blackhole/scope/watcher.hpp"

#include <boost/assert.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/scope/manager.hpp"

namespace blackhole {
inline namespace v1 {
namespace scope {

watcher_t::watcher_t(logger_t& logger) :
    manager(logger.manager()),
    prev(manager.get().get())
{
    manager.get().reset(this);
}

watcher_t::~watcher_t() {
    BOOST_ASSERT(manager.get().get() == this);
    manager.get().reset(prev);
}

auto watcher_t::collect(attribute_pack& pack) const -> void {
    pack.emplace_back(attributes());

    if (prev) {
        prev->collect(pack);
    }
}

auto watcher_t::rebind(manager_t& manager) -> void {
    this->manager = manager;

    if (prev) {
        prev->rebind(manager);
    }
}

}  // namespace scope
}  // namespace v1
}  // namespace blackhole
