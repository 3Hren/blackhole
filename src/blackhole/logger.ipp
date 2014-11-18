#include "blackhole/config.hpp"
#include "blackhole/detail/config/nullptr.hpp"
#include "blackhole/detail/thread/lock.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/trace/context.hpp"

#include "blackhole/keyword/process.hpp"

namespace blackhole {

template<class T, class... FilterArgs>
composite_logger_t<T, FilterArgs...>&
composite_logger_t<T, FilterArgs...>::operator=(composite_logger_t<T, FilterArgs...>&& other) {
    other.d.enabled = d.enabled.exchange(other.d.enabled);

    auto lock = detail::thread::multi_lock(d.lock.open, d.lock.push, other.d.lock.open, other.d.lock.push);

    using std::swap;
    swap(d.filter, other.d.filter);
    swap(d.frontends, other.d.frontends);

    auto this_scoped_attributes = scoped.get();
    scoped.reset(other.scoped.get());
    other.scoped.reset(this_scoped_attributes);

    if (scoped.get()) {
        scoped->m_logger = this;
    }

    if (other.scoped.get()) {
        other.scoped->m_logger = &other;
    }

    return *this;
}

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

template<class T, class... FilterArgs>
BLACKHOLE_API
void
composite_logger_t<T, FilterArgs...>::populate(attribute::set_t& external, const reader_lock_type&) const {
    external.reserve(BLACKHOLE_EXTERNAL_SET_RESERVED_SIZE);

    if (auto scoped = this->scoped.get()) {
        const auto& attributes = scoped->attributes();
        std::copy(attributes.begin(), attributes.end(), std::back_inserter(external));
    }
}

template<class T, class... FilterArgs>
BLACKHOLE_API
attribute::combined_view_t
composite_logger_t<T, FilterArgs...>::with_scoped(const attribute::set_t& external, const reader_lock_type&) const {
    if (auto scoped = this->scoped.get()) {
        return attribute::combined_view_t(external, scoped->attributes());
    } else {
        return attribute::combined_view_t(external);
    }
}

BLACKHOLE_API
scoped_attributes_concept_t::scoped_attributes_concept_t(scope_feature_t& log) :
    m_logger(&log),
    m_previous(log.scoped.get())
{
    log.scoped.reset(this);
}

BLACKHOLE_API
scoped_attributes_concept_t::~scoped_attributes_concept_t() {
    BOOST_ASSERT(m_logger);
    BOOST_ASSERT(m_logger->scoped.get() == this);
    m_logger->scoped.reset(m_previous);
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

} // namespace blackhole
