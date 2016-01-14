#pragma once

namespace blackhole {
inline namespace v1 {
namespace scope {

class watcher_t;

/// Represents a thread local storage of a scoped attributes watcher pointer.
class manager_t {
public:
    virtual ~manager_t() = 0;

    /// Returns the current scoped attributes watcher pointer value, nullptr otherwise.
    ///
    /// \note it's valid to return a nullptr as a scoped attributes watcher, meaning that there are
    ///     no scoped attributes registered before this call.
    virtual auto get() const -> watcher_t* = 0;

    /// Resets the current scoped attributes watch with the specified value.
    ///
    /// \param value scoped attributes watcher, may be nullptr.
    ///
    /// \post this->get() == value;
    virtual auto reset(watcher_t* value) -> void = 0;
};

}  // namespace scope
}  // namespace v1
}  // namespace blackhole
