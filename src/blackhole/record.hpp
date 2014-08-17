#pragma once

#include "attribute.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace log {

class record_t {
    attribute_set_view_t view;

public:
    /*!
     * Default constructor.
     * Creates an empty record that is equivalent to the invalid record handle.
     * @todo: Test post condition: this->valid() == false
     */
    record_t() = default;

    /*!
     * Conversion constructor.
     * Creates a record with specified attribute set.
     * @todo: Test post condition: this->attributes() == prev attributes.
     * @todo: Instead of `attribute::set_t` move `attribute_set_view_t`.
     */
    record_t(attribute_set_view_t&& view) :
        view(std::move(view))
    {}

    /*!
     * Conversion to an unspecified boolean type.
     * Return true if the record is valid.
     * @todo: Test.
     */
    operator bool() const BLACKHOLE_NOEXCEPT {
        return valid();
    }

    /*!
     * Check if the record is valid.
     * A record is considered valid if it contains at least one attribute.
     * @todo: Test.
     */
    bool valid() const BLACKHOLE_NOEXCEPT {
        return !view.empty();
    }

    /*!
     * Return a const reference to the view of attribute set attached to this
     * record.
     * @todo: Test.
     */
    const attribute_set_view_t& attributes() const BLACKHOLE_NOEXCEPT {
        return view;
    }

    /*!
     * Insert attribute pair into the record.
     * @todo: Test.
     */
    void insert(blackhole::attribute::pair_t pair) {
        view.insert(std::move(pair));
    }

    /*!
     * Try to extract attribute with specified name and convert it to type T.
     * @todo: What throws? Test invalid name and invalid type.
     */
    template<typename T>
    inline T extract(const std::string& name) const {
        return blackhole::attribute::traits<T>::extract(view, name);
    }
};

} // namespace log

} // namespace blackhole
