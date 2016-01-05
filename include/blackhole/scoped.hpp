#pragma once

#include "blackhole/logger.hpp"

namespace blackhole {

/// Represents scoped attributes guard.
///
/// Scoped attributes is the mechanism allowing to attach thread-local attributes to any logger
/// implementation until associated instance of this guard lives on the stack.
///
/// Internally scoped attributes are organized in a thread-local linked list ordering by least
/// living to most ones. This means, that the least attached attributes have more priority, but
/// they don't override each other, i.e duplicates are allowed.
///
/// Blackhole doesn't allow to create this object manually. Instead it can be obtained from any
/// logger implementation by calling `scoped(...)` method providing attributes list you want to
/// attach.
///
/// \warning explicit moving instances of this class will probably invoke an undefined behavior,
///     because it can violate construction/destruction order, which is strict.
///     However I can't just delete move constructor, because there won't be any way to return
///     objects from factory methods and that's the way they are created.
class scoped_t : public logger_t::scoped_t {
    attributes_t storage;
    attribute_list list;

public:
    /// Constructs a scoped guard which will attach the given attributes to the specified logger on
    /// construction making every further log event to contain them until keeped alive.
    ///
    /// Creating multiple scoped guards results in attributes stacking.
    scoped_t(logger_t& logger, attributes_t attributes);

    /// Copying scoped guards is deliberately prohibited.
    scoped_t(const scoped_t& other) = delete;

    /// Move constructor is left default for enabling copy elision. It never takes place in fact.
    ///
    /// \warning you should never move scoped guard instances manually, otherwise the behavior is
    ///     undefined.
    scoped_t(scoped_t&& other) = default;

    /// Assignment is deliberately prohibited.
    auto operator=(const scoped_t& other) -> scoped_t& = delete;
    auto operator=(scoped_t&& other) -> scoped_t& = delete;

    auto attributes() const -> const attribute_list&;
};

}  // namespace blackhole
