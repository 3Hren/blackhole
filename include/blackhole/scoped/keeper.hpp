#pragma once

#include "blackhole/scoped.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"

namespace blackhole {
namespace scoped {

/// Implementation of scoped attributes guard that keeps attributes provided on construction.
class keeper_t : public scoped_t {
    attributes_t storage;
    attribute_list list;

public:
    /// Constructs a scoped guard which will attach the given attributes to the specified logger on
    /// construction making every further log event to contain them until keeped alive.
    ///
    /// \note creating multiple scoped guards results in attributes stacking.
    keeper_t(logger_t& logger, attributes_t attributes);

    // TODO: Try to delete these. Rule of zero.
    /// Copying scoped guards is deliberately prohibited.
    keeper_t(const keeper_t& other) = delete;

    /// Move constructor is left default for enabling copy elision. It never takes place in fact.
    ///
    /// \warning you should never move scoped guard instances manually, otherwise the behavior is
    ///     undefined.
    keeper_t(keeper_t&& other) = default;

    /// Assignment is deliberately prohibited.
    auto operator=(const keeper_t& other) -> keeper_t& = delete;
    auto operator=(keeper_t&& other) -> keeper_t& = delete;

    /// Returns an immutable reference to the internal attribute list.
    auto attributes() const -> const attribute_list&;
};

}  // namespace scoped
}  // namespace blackhole
