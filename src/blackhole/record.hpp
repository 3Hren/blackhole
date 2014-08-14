#pragma once

#include "attribute.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace log {

class record_t {
    //!@todo: Here will be non-const `attribute_set_view_t`.
    attributes_t attributes_;

public:
    /*!
     * Default constructor.
     * Creates an empty record that is equivalent to the invalid record handle.
     * @todo: Test post condition: this->valid() == false
     */
    record_t() = default;

    //!@todo: Instead of `attributes_t` move `attribute_set_view_t`.
    record_t(attributes_t&& attributes) :
        attributes_(std::move(attributes))
    {}

    operator bool() const noexcept {
        return valid();
    }

    bool valid() const noexcept {
        return !attributes_.empty();
    }

    /*!
     * Return a const reference to the view of attribute set attached to this
     * record.
     */
    const log::attributes_t& attributes() const noexcept {
        return attributes_;
    }

    //!@todo: Refactor.
    template<typename T, typename... Args>
    inline void fill(T pair, Args&&... args) {
        insert(pair);
        fill(std::forward<Args>(args)...);
    }

    inline void fill() {}

    void insert(const attribute_pair_t& pair) {
        attributes_.insert(pair);
    }

    void insert(attribute_pair_t&& pair) {
        attributes_.insert(std::move(pair));
    }

    template<typename T>
    inline T extract(const std::string& name) const {
        return blackhole::attribute::traits<T>::extract(attributes_, name);
    }
};

} // namespace log

} // namespace blackhole
