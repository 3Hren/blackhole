#pragma once

#include "blackhole/config.hpp"
#include "blackhole/trace/context.hpp"
#include "blackhole/utils/noexcept.hpp"

#ifdef __linux__
# define BLACKHOLE_HAS_LWP
# include <sys/syscall.h>
#endif

namespace blackhole {

namespace aux {

namespace guard {

inline void no_deleter(scoped_attributes_concept_t*) { }

} // namespace guard

} //namespace detail

BLACKHOLE_DECL
logger_base_t::logger_base_t() :
    m_enabled(true),
    tracked_(false), //!@todo: Test - is false dy default.
    m_filter(default_filter_t::instance()),
    m_exception_handler(log::default_exception_handler_t()),
    m_scoped_attributes(&aux::guard::no_deleter)
{}

BLACKHOLE_DECL
logger_base_t::logger_base_t(logger_base_t&& other) BLACKHOLE_NOEXCEPT :
    m_scoped_attributes(&aux::guard::no_deleter)
{
    *this = std::move(other);
}

BLACKHOLE_DECL
logger_base_t&
logger_base_t::operator=(logger_base_t&& other) BLACKHOLE_NOEXCEPT {
    swap(*this, other);
    return *this;
}

BLACKHOLE_DECL
bool
logger_base_t::enabled() const {
    return m_enabled;
}

BLACKHOLE_DECL
void
logger_base_t::enable() {
    m_enabled = true;
}

BLACKHOLE_DECL
void
logger_base_t::disable() {
    m_enabled = false;
}

BLACKHOLE_DECL
void
logger_base_t::set_filter(filter_t&& filter) {
    m_filter = std::move(filter);
}

BLACKHOLE_DECL
void
logger_base_t::add_attribute(const log::attribute_pair_t& attribute) {
    m_global_attributes.insert(attribute);
}

BLACKHOLE_DECL
void
logger_base_t::add_frontend(std::unique_ptr<base_frontend_t> frontend) {
    m_frontends.push_back(std::move(frontend));
}

BLACKHOLE_DECL
void
logger_base_t::set_exception_handler(log::exception_handler_t&& handler) {
    m_exception_handler = std::move(handler);
}

BLACKHOLE_DECL
void
logger_base_t::track(bool enable) {
    this->tracked_ = enable;
}

BLACKHOLE_DECL
log::record_t
logger_base_t::open_record() const {
    return open_record(log::attributes_t());
}

BLACKHOLE_DECL
log::record_t
logger_base_t::open_record(log::attribute_pair_t local_attribute) const {
    return open_record(log::attributes_t({ std::move(local_attribute) }));
}

BLACKHOLE_DECL
log::record_t
logger_base_t::open_record(log::attributes_t local_attributes) const {
    if (enabled() && !m_frontends.empty()) {
        log::attributes_t trace_attributes;
        if (tracked_) {
            trace_attributes.insert(
                attribute::make("trace", ::this_thread::current_span().trace)
            );
        }

        log::attributes_t attributes = merge({
            universe_storage_t::instance().dump(),  // Program global.
            get_thread_attributes(),                // Thread local.
            m_global_attributes,                    // Logger object specific.
            get_event_attributes(),                 // Event specific, e.g. timestamp.
            std::move(local_attributes),            // Any user attributes.
            m_scoped_attributes.get() ? m_scoped_attributes->attributes() : log::attributes_t(),
            trace_attributes
        });

        if (m_filter(attributes)) {
            log::record_t record;
            record.attributes = std::move(attributes);
            return record;
        }
    }

    return log::record_t();
}

BLACKHOLE_DECL
void
logger_base_t::push(log::record_t&& record) const {
    for (auto it = m_frontends.begin(); it != m_frontends.end(); ++it) {
        try {
            const std::unique_ptr<base_frontend_t>& frontend = *it;
            frontend->handle(record);
        } catch (...) {
            m_exception_handler();
        }
    }
}

BLACKHOLE_DECL
log::attributes_t
logger_base_t::get_event_attributes() const {
    timeval tv;
    gettimeofday(&tv, nullptr);
    log::attributes_t attributes = {
        keyword::timestamp() = tv
    };
    return attributes;
}

BLACKHOLE_DECL
log::attributes_t
logger_base_t::get_thread_attributes() const {
    log::attributes_t attributes = {
#ifdef BLACKHOLE_HAS_LWP
        keyword::lwp() = syscall(SYS_gettid)
#else
        keyword::tid() = this_thread::get_id<std::string>()
#endif
    };
    return attributes;
}

BLACKHOLE_DECL
scoped_attributes_concept_t::scoped_attributes_concept_t(logger_base_t& log) :
    m_logger(&log),
    m_previous(log.m_scoped_attributes.get())
{
    log.m_scoped_attributes.reset(this);
}

BLACKHOLE_DECL
scoped_attributes_concept_t::~scoped_attributes_concept_t() {
    BOOST_ASSERT(m_logger);
    BOOST_ASSERT(m_logger->m_scoped_attributes.get() == this);
    m_logger->m_scoped_attributes.reset(m_previous);
}

BLACKHOLE_DECL
bool
scoped_attributes_concept_t::has_parent() const {
    return m_previous;
}

BLACKHOLE_DECL
const scoped_attributes_concept_t&
scoped_attributes_concept_t::parent() const {
    return *m_previous;
}

BLACKHOLE_DECL
void
swap(logger_base_t& lhs, logger_base_t& rhs) BLACKHOLE_NOEXCEPT {
    using std::swap;
    swap(lhs.m_enabled, rhs.m_enabled);
    swap(lhs.tracked_, rhs.tracked_); //!@todo: Test - is swapped
    swap(lhs.m_filter, rhs.m_filter);
    swap(lhs.m_exception_handler, rhs.m_exception_handler);
    swap(lhs.m_frontends, rhs.m_frontends);
    swap(lhs.m_global_attributes, rhs.m_global_attributes);

    auto lhs_operation_attributes = lhs.m_scoped_attributes.get();
    lhs.m_scoped_attributes.reset(rhs.m_scoped_attributes.get());
    rhs.m_scoped_attributes.reset(lhs_operation_attributes);

    if (lhs.m_scoped_attributes.get()) {
        lhs.m_scoped_attributes->m_logger = &lhs;
    }

    if (rhs.m_scoped_attributes.get()) {
        rhs.m_scoped_attributes->m_logger = &rhs;
    }
}

} // namespace blackhole
