#include "blackhole/config.hpp"
#include "blackhole/detail/thread/lock.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/platform.hpp"
#include "blackhole/trace/context.hpp"
#include "blackhole/utils/noexcept.hpp"

#ifdef __linux__
# define BLACKHOLE_HAS_LWP
# include <sys/syscall.h>
#endif

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
    filter(default_filter_t::instance()),
    attributes(&aux::guard::no_deleter),
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
logger_base_t::add_attribute(const attribute::pair_t& attribute) {
    writer_lock_type lock(state.lock.open);
    state.attributes.global.insert(attribute);
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
logger_base_t::open_record(attribute::set_t attributes) const {
    if (enabled() && !state.frontends.empty()) {
        // attributes.insert(universe_storage_t::instance().dump()); // Program global.
        attributes.insert(
#ifdef BLACKHOLE_HAS_LWP
            keyword::lwp() = syscall(SYS_gettid)
#else
            keyword::tid() = this_thread::get_id<std::string>()
#endif
        );

        timeval tv;
        gettimeofday(&tv, nullptr);
        attributes.insert(keyword::timestamp() = tv);

        if (state.tracked) {
            attributes.insert(
                attribute::make("trace", ::this_thread::current_span().trace)
            );
        }

        reader_lock_type lock(state.lock.open);
        attribute::set_view_t set(
            state.attributes.global,
            state.attributes.scoped.get() ?
                state.attributes.scoped->attributes() :
                attribute::set_t(),
            std::move(attributes)
        );

        if (state.filter(set)) {
            return record_t(std::move(set));
        }
    }

    return record_t();
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
    m_previous(log.state.attributes.scoped.get())
{
    log.state.attributes.scoped.reset(this);
}

BLACKHOLE_API
scoped_attributes_concept_t::~scoped_attributes_concept_t() {
    BOOST_ASSERT(m_logger);
    BOOST_ASSERT(m_logger->state.attributes.scoped.get() == this);
    m_logger->state.attributes.scoped.reset(m_previous);
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
    swap(lhs.state.attributes.global, rhs.state.attributes.global);

    swap(lhs.state.frontends, rhs.state.frontends);
    swap(lhs.state.exception, rhs.state.exception);

    auto lhs_operation_attributes = lhs.state.attributes.scoped.get();
    lhs.state.attributes.scoped.reset(rhs.state.attributes.scoped.get());
    rhs.state.attributes.scoped.reset(lhs_operation_attributes);

    if (lhs.state.attributes.scoped.get()) {
        lhs.state.attributes.scoped->m_logger = &lhs;
    }

    if (rhs.state.attributes.scoped.get()) {
        rhs.state.attributes.scoped->m_logger = &rhs;
    }
}

} // namespace blackhole

//!@todo: Concept. AttributeSet - own map with impl to be able to move faster.
//!       Impl - boost::array<16> + list. Located on shared pool.
//!       Each node is located on another shared pool.
