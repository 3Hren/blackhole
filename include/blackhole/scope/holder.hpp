#pragma once

#include "blackhole/scope/watcher.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"

namespace blackhole {
inline namespace v1 {
namespace scope {

/// Implementation of scoped attributes guard that keeps attributes provided on construction and
/// provides them each time on demand.
class holder_t : public watcher_t {
    attributes_t storage;
    attribute_list list;

public:
    /// Constructs a scoped guard which will attach the given attributes to the specified logger on
    /// construction making every further log event to contain them until keeped alive.
    ///
    /// \note creating multiple scoped watch objects results in attributes stacking.
    holder_t(logger_t& logger, attributes_t attributes);

    /// Returns an immutable reference to the internal attribute list.
    auto attributes() const -> const attribute_list&;
};

}  // namespace scoped
}  // namespace v1
}  // namespace blackhole
