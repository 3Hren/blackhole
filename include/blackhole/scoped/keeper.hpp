#pragma once

#include "blackhole/scoped.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"

namespace blackhole {
namespace scoped {

/// Represents scoped attributes guard.
///
/// Scoped attributes is the mechanism allowing to attach thread-local attributes to any logger
/// implementation until associated instance of this guard lives on the stack.
///
/// Internally scoped attributes are organized in a thread-local linked list ordering by least
/// living to most ones. This means, that the least attached attributes have more priority, but they
/// don't override each other, i.e duplicates are allowed.
///
/// \warning explicit moving instances of this class will probably invoke an undefined behavior,
///     because it can violate construction/destruction order, which is strict.
///     However I can't just delete move constructor, because there won't be any way to return
///     objects from factory methods and that's the way they are created.
class keeper_t : public scoped_t {
    attributes_t storage;
    attribute_list list;

public:
    /// Constructs a scoped guard which will attach the given attributes to the specified logger on
    /// construction making every further log event to contain them until keeped alive.
    ///
    /// Creating multiple scoped guards results in attributes stacking.
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

    auto attributes() const -> const attribute_list&;
};

}  // namespace scoped
}  // namespace blackhole
