#include "blackhole/logger.hpp"

#include "blackhole/detail/config/nullptr.hpp"
#include "blackhole/keyword.hpp"
#include "blackhole/keyword/message.hpp"
#include "blackhole/keyword/thread.hpp"
#include "blackhole/keyword/timestamp.hpp"
#include "blackhole/keyword/tracebit.hpp"
#include "blackhole/keyword/process.hpp"

namespace blackhole {

template<class T, class... FilterArgs>
void
composite_logger_t<T, FilterArgs...>::populate(attribute::set_t& internal) const {
    internal.reserve(BLACKHOLE_INTERNAL_SET_RESERVED_SIZE);

#ifdef BLACKHOLE_HAS_ATTRIBUTE_PID
    internal.emplace_back(keyword::pid() = keyword::init::pid());
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_TID
    internal.emplace_back(keyword::tid() = keyword::init::tid());
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_LWP
    internal.emplace_back(keyword::lwp() = keyword::init::lwp());
#endif

    internal.emplace_back(keyword::timestamp() = keyword::init::timestamp());
}

BLACKHOLE_API
scoped_attributes_concept_t::~scoped_attributes_concept_t() {
    BOOST_ASSERT(m_scoped);
    BOOST_ASSERT(m_scoped->scoped.get() == this);
    m_scoped->scoped.reset(m_previous);
}

BLACKHOLE_API
bool
scoped_attributes_concept_t::has_parent() const {
    return m_previous;
}

BLACKHOLE_API
const scoped_attributes_concept_t&
scoped_attributes_concept_t::parent() const {
    return *m_previous;
}

BLACKHOLE_API
void scoped_feature_t::swap(scoped_feature_t &other) {
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
void scoped_feature_t::merge(attribute::set_t &external) const {
    if (auto scoped = this->scoped.get()) {
        const auto& attributes = scoped->attributes();
        std::copy(attributes.begin(), attributes.end(), std::back_inserter(external));
    }
}

BLACKHOLE_API
attribute::combined_view_t scoped_feature_t::view(const attribute::set_t &external) const {
    if (auto scoped = this->scoped.get()) {
        return attribute::combined_view_t(external, scoped->attributes());
    } else {
        return attribute::combined_view_t(external);
    }
}

} // namespace blackhole
