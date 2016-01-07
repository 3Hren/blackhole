#pragma once

#include "blackhole/scoped.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"

namespace blackhole {
namespace scoped {

/// Implementation of scoped attributes guard that keeps attributes provided on construction and
/// provides them each time on demand.
class keeper_t : public scoped_t {
    attributes_t storage;
    attribute_list list;

public:
    /// Constructs a scoped guard which will attach the given attributes to the specified logger on
    /// construction making every further log event to contain them until keeped alive.
    ///
    /// \note creating multiple scoped guards results in attributes stacking.
    keeper_t(logger_t& logger, attributes_t attributes);

    /// Returns an immutable reference to the internal attribute list.
    auto attributes() const -> const attribute_list&;
};

}  // namespace scoped
}  // namespace blackhole
