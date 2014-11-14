#include "blackhole/config.hpp"
#include "blackhole/detail/config/nullptr.hpp"
#include "blackhole/detail/thread/lock.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/trace/context.hpp"

#include "blackhole/keyword/process.hpp"

namespace blackhole {

namespace aux {

namespace guard {

inline void no_deleter(scoped_attributes_concept_t*) {}

} // namespace guard

} //namespace aux

BLACKHOLE_API
logger_base_t::state_t::state_t() :
    enabled(true),
    tracked(false),
    filter(&filter::none),
    scoped(&aux::guard::no_deleter),
    exception(log::default_exception_handler_t())
{}

BLACKHOLE_API
logger_base_t::logger_base_t()
{}

BLACKHOLE_API
logger_base_t::logger_base_t(logger_base_t&& other) BLACKHOLE_NOEXCEPT
{
    *this = std::move(other);
}

BLACKHOLE_API
logger_base_t&
logger_base_t::operator=(logger_base_t&& other) BLACKHOLE_NOEXCEPT {
    swap(*this, other);
    return *this;
}

BLACKHOLE_API
bool
logger_base_t::enabled() const {
    return state.enabled;
}

BLACKHOLE_API
void
logger_base_t::enabled(bool enable) {
    state.enabled = enable;
}

BLACKHOLE_API
bool
logger_base_t::tracked() const {
    return state.tracked;
}

BLACKHOLE_API
void
logger_base_t::tracked(bool enable) {
    state.tracked = enable;
}

BLACKHOLE_API
void
logger_base_t::set_filter(filter_t&& filter) {
    writer_lock_type lock(state.lock.open);
    state.filter = std::move(filter);
}

BLACKHOLE_API
void
logger_base_t::add_frontend(std::unique_ptr<base_frontend_t> frontend) {
    writer_lock_type lock(state.lock.push);
    state.frontends.push_back(std::move(frontend));
}

BLACKHOLE_API
void
logger_base_t::set_exception_handler(log::exception_handler_t&& handler) {
    writer_lock_type lock(state.lock.push);
    state.exception = std::move(handler);
}

BLACKHOLE_API
record_t
logger_base_t::open_record() const {
    return open_record(attribute::set_t());
}

BLACKHOLE_API
record_t
logger_base_t::open_record(attribute::pair_t attribute) const {
    return open_record(attribute::set_t({ std::move(attribute) }));
}

BLACKHOLE_API
record_t
logger_base_t::open_record(attribute::set_t external) const {
    reader_lock_type lock(state.lock.open);
    if (enabled() && !state.frontends.empty()) {

        const attribute::combined_view_t view = with_scoped(external, lock);
        if (state.filter(view)) {
            attribute::set_t internal;
            populate(internal);
            populate(external, lock);
            return record_t(std::move(internal), std::move(external));
        }
    }

    return record_t::invalid();
}

BLACKHOLE_API
void
logger_base_t::populate(attribute::set_t& internal) const {
    internal.reserve(6); // TODO: Magic.

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

    if (state.tracked) {
        internal.emplace_back(attribute::make("trace", ::this_thread::current_span().trace));
    }
}

BLACKHOLE_API
void
logger_base_t::populate(attribute::set_t& external, const reader_lock_type&) const {
    external.reserve(16); // TODO: Magic.

    if (auto scoped = state.scoped.get()) {
        const auto& attributes = scoped->attributes();
        std::copy(attributes.begin(), attributes.end(), std::back_inserter(external));
    }
}

BLACKHOLE_API
attribute::combined_view_t
logger_base_t::with_scoped(const attribute::set_t& external, const reader_lock_type&) const {
    if (auto scoped = state.scoped.get()) {
        return attribute::combined_view_t(external, scoped->attributes());
    } else {
        return attribute::combined_view_t(external);
    }
}

BLACKHOLE_API
void
logger_base_t::push(record_t&& record) const {
    reader_lock_type lock(state.lock.push);
    for (auto it = state.frontends.begin(); it != state.frontends.end(); ++it) {
        try {
            const std::unique_ptr<base_frontend_t>& frontend = *it;
            frontend->handle(record);
        } catch (...) {
            state.exception();
        }
    }
}

BLACKHOLE_API
scoped_attributes_concept_t::scoped_attributes_concept_t(logger_base_t& log) :
    m_logger(&log),
    m_previous(log.state.scoped.get())
{
    log.state.scoped.reset(this);
}

BLACKHOLE_API
scoped_attributes_concept_t::~scoped_attributes_concept_t() {
    BOOST_ASSERT(m_logger);
    BOOST_ASSERT(m_logger->state.scoped.get() == this);
    m_logger->state.scoped.reset(m_previous);
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
void
swap(logger_base_t& lhs, logger_base_t& rhs) BLACKHOLE_NOEXCEPT {
    rhs.state.enabled = lhs.state.enabled.exchange(rhs.state.enabled);
    rhs.state.tracked = lhs.state.tracked.exchange(rhs.state.tracked);

    auto lock = detail::thread::make_multi_lock_t(
        lhs.state.lock.open,
        lhs.state.lock.push,
        rhs.state.lock.open,
        rhs.state.lock.push
    );

    using std::swap;
    swap(lhs.state.filter, rhs.state.filter);

    swap(lhs.state.frontends, rhs.state.frontends);
    swap(lhs.state.exception, rhs.state.exception);

    auto lhs_operation_attributes = lhs.state.scoped.get();
    lhs.state.scoped.reset(rhs.state.scoped.get());
    rhs.state.scoped.reset(lhs_operation_attributes);

    if (lhs.state.scoped.get()) {
        lhs.state.scoped->m_logger = &lhs;
    }

    if (rhs.state.scoped.get()) {
        rhs.state.scoped->m_logger = &rhs;
    }
}

} // namespace blackhole
