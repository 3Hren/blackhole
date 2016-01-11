#pragma once

#include "blackhole/attributes.hpp"

namespace boost {
    template<typename> class thread_specific_ptr;
}  // namespace boost

namespace blackhole {
inline namespace v1 {

class logger_t;

/// Represents scoped attributes guard interface.
///
/// Scoped attributes is the mechanism allowing to attach thread-local attributes to any logger
/// implementation until associated instance of this guard lives on the stack.
///
/// Internally scoped attributes are organized in a thread-local linked list ordering by least
/// living to most ones. This means, that the least attached attributes have more priority, but
/// they don't override each other, i.e duplicates are allowed.
///
/// \warning explicit moving instances of this class will probably invoke an undefined behavior,
///     because it can violate construction/destruction order, which is strict.
class scoped_t {
    boost::thread_specific_ptr<scoped_t>* context;
    scoped_t* prev;

public:
    /// Constructs a scoped attributes guard which will be associated with the specified logger.
    explicit scoped_t(logger_t& logger);

    /// Both copy and move construction are deliberately prohibited.
    scoped_t(const scoped_t& other) = delete;
    scoped_t(scoped_t&& other) = delete;

    /// Destroys the current scoped guard with popping early attached attributes from the scoped
    /// attributes stack.
    ///
    /// \warning scoped guards **must** be destroyed in reversed order they were created,
    ///     otherwise the behavior is undefined.
    virtual ~scoped_t();

    /// Both copy and move assignment are deliberately prohibited.
    auto operator=(const scoped_t& other) -> scoped_t& = delete;
    auto operator=(scoped_t&& other) -> scoped_t& = delete;

    /// Recursively collects all scoped attributes into the given attributes pack.
    auto collect(attribute_pack* pack) const -> void;

    /// Recursively rebind all scoped attributes with the new logger context.
    auto rebind(boost::thread_specific_ptr<scoped_t>* context) -> void;

    /// Returns an immutable reference to the internal attribute list.
    virtual auto attributes() const -> const attribute_list& = 0;
};

}  // namespace v1
}  // namespace blackhole
