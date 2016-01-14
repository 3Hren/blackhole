#pragma once

#include "blackhole/attributes.hpp"

namespace blackhole {
inline namespace v1 {

class logger_t;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace scope {

class manager_t;

/// Represents scoped attributes watch interface.
///
/// Scoped attributes is the mechanism allowing to attach thread-local attributes to any logger
/// implementation until associated instance of this watch lives on the stack.
///
/// Internally scoped attributes are organized in a thread-local linked list ordering by least
/// living to most ones. This means, that the least attached attributes have more priority, but
/// they don't override each other, i.e duplicates are allowed.
///
/// \warning explicit moving instances of this class will probably invoke an undefined behavior,
///     because it can violate construction/destruction order, which is strict.
class watcher_t {
    std::reference_wrapper<manager_t> manager;
    watcher_t* prev;

public:
    /// Constructs a scoped attributes watch which will be associated with the specified logger.
    explicit watcher_t(logger_t& logger);

    /// Both copy and move construction are deliberately prohibited.
    watcher_t(const watcher_t& other) = delete;
    watcher_t(watcher_t&& other) = delete;

    /// Destroys the current scoped watch with popping early attached attributes from the scoped
    /// attributes stack.
    ///
    /// \warning scoped watch objects **must** be destroyed in reversed order they were created,
    ///     otherwise the behavior is undefined.
    virtual ~watcher_t();

    /// Both copy and move assignment are deliberately prohibited.
    auto operator=(const watcher_t& other) -> watcher_t& = delete;
    auto operator=(watcher_t&& other) -> watcher_t& = delete;

    /// Recursively collects all scoped attributes into the given attributes pack.
    auto collect(attribute_pack& pack) const -> void;

    /// Recursively rebind all scoped attributes with the new logger manager.
    ///
    /// Usually called in the middle of the logger's move operation.
    auto rebind(manager_t& manager) -> void;

    /// Returns an immutable reference to the internal attribute list.
    virtual auto attributes() const -> const attribute_list& = 0;
};

}  // namespace scope
}  // namespace v1
}  // namespace blackhole
