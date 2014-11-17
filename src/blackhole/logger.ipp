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
    auto& lhs = *this;
    auto& rhs = other;

    rhs.state.enabled = lhs.state.enabled.exchange(rhs.state.enabled);

    auto lock = blackhole::detail::thread::make_multi_lock_t(
        lhs.state.lock.open,
        lhs.state.lock.push,
        rhs.state.lock.open,
        rhs.state.lock.push
    );

    using std::swap;
    swap(lhs.state.filter, rhs.state.filter);

    swap(lhs.state.frontends, rhs.state.frontends);

    auto lhs_operation_attributes = lhs.scoped.get();
    lhs.scoped.reset(rhs.scoped.get());
    rhs.scoped.reset(lhs_operation_attributes);

    if (lhs.scoped.get()) {
        lhs.scoped->m_logger = &lhs;
    }

    if (rhs.scoped.get()) {
        rhs.scoped->m_logger = &rhs;
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
