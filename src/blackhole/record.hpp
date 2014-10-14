#pragma once

#include "attribute.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

class record_t {
    attribute::set_view_t view;

public:
    /*!
     * Default constructor.
     * Creates an empty record that is equivalent to the invalid record handle.
     */
    record_t() = default;

    /*!
     * Conversion constructor.
     * Creates a record with specified attribute set.
     */
    record_t(attribute::set_view_t&& view) :
        view(std::move(view))
    {}

    /*!
     * Conversion to an unspecified boolean type.
     * Return true if the record is valid.
     */
    operator bool() const BLACKHOLE_NOEXCEPT {
        return valid();
    }

    /*!
     * Check if the record is valid.
     * A record is considered valid if it contains at least one attribute.
     */
    bool valid() const BLACKHOLE_NOEXCEPT {
        return !view.empty();
    }

    /*!
     * Return a const reference to the view of attribute set attached to this
     * record.
     */
    const attribute::set_view_t& attributes() const BLACKHOLE_NOEXCEPT {
        return view;
    }

    /*!
     * Set message attribute to the internal attribute set.
     * This can be helpful, because message formatting can take significant
     * amount of time to provide it while opening record.
     */
    void message(const std::string& message) {
        view.message(message);
    }

    void message(std::string&& message) {
        view.message(std::move(message));
    }

    /*!
     * Insert attribute pair into the record.
     */
    void insert(attribute::pair_t pair) {
        view.insert(std::move(pair));
    }

    /*!
     * Try to extract attribute with specified name and convert it to type T.
     * @throw: std::out_of_range - if attribute with specified name was not
     *         found in the record.
     * @throw: boost::bad_get - if attribute with specified name was found in
     *         the record, but it has other underlying type than specified one.
     */
    template<typename T>
    T extract(const std::string& name) const {
        return attribute::traits<T>::extract(view, name);
    }
};

namespace log {

typedef blackhole::record_t record_t BLACKHOLE_DEPRECATED("Use `record_t` instead.");

} // namespace log

} // namespace blackhole
