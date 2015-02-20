#pragma once

#include "blackhole/config.hpp"

#include "attribute.hpp"
#include "blackhole/utils/format.hpp"

BLACKHOLE_BEG_NS

class record_t {
    bool valid_;
    attribute::set_view_t view;

public:
    /*!
     * Default constructor.
     * Creates an empty record.
     */
    record_t() : valid_(true) {}

    /*!
     * Conversion constructor.
     * Creates a record with specified attribute set.
     */
    explicit record_t(attribute::set_view_t&& view) :
        valid_(true),
        view(std::move(view))
    {}

    /*!
     * Return invalid record.
     */
    static record_t invalid() {
        record_t invalid;
        invalid.valid_ = false;
        return invalid;
    }

    /*!
     * Conversion to an unspecified boolean type.
     * Return true if the record is valid.
     */
    operator bool() const BLACKHOLE_NOEXCEPT {
        return valid();
    }

    /*!
     * Checks if the record is valid.
     */
    bool valid() const BLACKHOLE_NOEXCEPT {
        return valid_;
    }

    /*!
     * Return a const reference to the view of attribute set attached to this
     * record.
     */
    const attribute::set_view_t& attributes() const BLACKHOLE_NOEXCEPT {
        BOOST_ASSERT(valid());
        return view;
    }

    /*!
     * Set message attribute to the internal attribute set.
     * This can be helpful, because message formatting can take significant
     * amount of time to provide it while opening record.
     */
    void message(const std::string& message) {
        BOOST_ASSERT(valid());
        view.message(message);
    }

    void message(std::string&& message) {
        BOOST_ASSERT(valid());
        view.message(std::move(message));
    }

    /*!
     * Insert attribute pair into the record.
     */
    void insert(attribute::pair_t pair) {
        BOOST_ASSERT(valid());
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
        BOOST_ASSERT(valid());
        return attribute::traits<T>::extract(view, name);
    }
};

namespace log {

typedef blackhole::record_t record_t BLACKHOLE_DEPRECATED("Use `record_t` instead.");

} // namespace log

BLACKHOLE_END_NS
