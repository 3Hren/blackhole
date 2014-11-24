#pragma once

#include <iterator>

#include <boost/thread/tss.hpp>

#include "blackhole/attribute/set.hpp"
#include "blackhole/config.hpp"
#include "blackhole/detail/config/noncopyable.hpp"
#include "blackhole/forwards.hpp"

namespace blackhole {

namespace feature {

/*!
 * This feature allows the logger to be attached with scoped attributes.
 *
 * \note Use only as template parameter.
 */
class scoped_t {
    boost::thread_specific_ptr<scoped_attributes_concept_t> scoped;

public:
    scoped_t() :
        scoped(&scoped_t::empty_deleter)
    {}

    void swap(scoped_t& other);

    void merge(attribute::set_t& external) const;

    attribute::combined_view_t view(const attribute::set_t& external) const;

    scoped_attributes_concept_t* get() const {
        return scoped.get();
    }

    void reset(scoped_attributes_concept_t* value) {
        scoped.reset(value);
    }

private:
    static inline void empty_deleter(scoped_attributes_concept_t*) {}
};

} // namespace feature

} // namespace blackhole

//==================================================================================================

namespace blackhole {

/*!
 * Concept form scoped attributes holder.
 *
 * \note It's not movable to avoid moving to another thread.
 */
class scoped_attributes_concept_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(scoped_attributes_concept_t);

    feature::scoped_t *m_scoped;
    scoped_attributes_concept_t *m_previous;

    friend class feature::scoped_t;

public:
    template<class T>
    scoped_attributes_concept_t(T& log, typename T::scoped_type* = 0) :
        m_scoped(&log.scoped),
        m_previous(log.scoped.get())
    {
        log.scoped.reset(this);
    }

    virtual ~scoped_attributes_concept_t() {
        BOOST_ASSERT(m_scoped);
        BOOST_ASSERT(m_scoped->get() == this);
        m_scoped->reset(m_previous);
    }

    virtual const attribute::set_t& attributes() const = 0;

protected:
    bool has_parent() const {
        return m_previous;
    }

    const scoped_attributes_concept_t& parent() const {
        return *m_previous;
    }
};

} // namespace blackhole

//==================================================================================================

namespace blackhole {

namespace feature {

BLACKHOLE_API
void scoped_t::swap(scoped_t &other) {
    auto this_scoped_attributes = scoped.get();
    scoped.reset(other.scoped.get());
    other.scoped.reset(this_scoped_attributes);

    if (scoped.get()) {
        scoped->m_scoped = this;
    }

    if (other.scoped.get()) {
        other.scoped->m_scoped = &other;
    }
}

BLACKHOLE_API
void scoped_t::merge(attribute::set_t& external) const {
    if (auto scoped = this->scoped.get()) {
        const auto& attributes = scoped->attributes();
        std::copy(attributes.begin(), attributes.end(), std::back_inserter(external));
    }
}

BLACKHOLE_API
attribute::combined_view_t scoped_t::view(const attribute::set_t& external) const {
    if (auto scoped = this->scoped.get()) {
        return attribute::combined_view_t(external, scoped->attributes());
    } else {
        return attribute::combined_view_t(external);
    }
}

} // namespace feature

} // namespace blackhole
