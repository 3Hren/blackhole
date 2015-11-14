#pragma once

#include "blackhole/attributes.hpp"

namespace boost {
    template<typename> class thread_specific_ptr;
}  // namespace boost

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
class scoped_t {
    scoped_t* prev;
    boost::thread_specific_ptr<scoped_t>* context;

protected:
    attributes_t storage;
    attribute_list attributes;

public:
    scoped_t(boost::thread_specific_ptr<scoped_t>* context, attributes_t attributes);

    scoped_t(const scoped_t& other) = delete;
    scoped_t(scoped_t&& other) = default;

    /// Destroys the current scoped guard with popping early attached attributes from the scoped
    /// attributes stack.
    ///
    /// \warning scoped guards **must** be destroyed in reversed order they were created, otherwise
    ///     the behavior is undefined.
    ~scoped_t();

    /// Assignment is deliberately prohibited.
    auto operator=(const scoped_t& other) -> scoped_t& = delete;
    auto operator=(scoped_t&& other) -> scoped_t& = delete;

    auto collect(attribute_pack* pack) const -> void {
        pack->emplace_back(attributes);

        if (prev) {
            prev->collect(pack);
        }
    }

    auto rebind(boost::thread_specific_ptr<scoped_t>* context) -> void {
        this->context = context;
        if (prev) {
            prev->rebind(context);
        }
    }
};

}  // namespace blackhole
