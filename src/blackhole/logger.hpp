#pragma once

#include <memory>
#include <vector>

#include <boost/thread/tss.hpp>

#include "attribute.hpp"
#include "common.hpp"
#include "error/handler.hpp"
#include "filter.hpp"
#include "frontend.hpp"
#include "keyword.hpp"
#include "keyword/message.hpp"
#include "keyword/severity.hpp"
#include "keyword/thread.hpp"
#include "keyword/timestamp.hpp"
#include "keyword/tracebit.hpp"
#include "universe.hpp"
#include "utils/noncopyable.hpp"
#include "utils/unique.hpp"

#include "blackhole/config.hpp"
#include "blackhole/utils/atomic.hpp"
#include "blackhole/utils/noexcept.hpp"

namespace blackhole {

class scoped_attributes_concept_t;

template <typename Level>
struct logger_verbosity_traits {
    typedef Level level_type;

    static
    bool
    passed(level_type logger_verbosity, level_type record_verbosity) {
        typedef typename aux::underlying_type<Level>::type underlying_type;

        return static_cast<underlying_type>(record_verbosity) >=
            static_cast<underlying_type>(logger_verbosity);
    }
};

class logger_base_t {
protected:
    typedef boost::shared_mutex rw_mutex_type;
    typedef boost::shared_lock<rw_mutex_type> reader_lock_type;
    typedef boost::unique_lock<rw_mutex_type> writer_lock_type;

    struct state_t {
        std::atomic<bool> enabled;
        std::atomic<bool> tracked;

        filter_t filter;
        struct attrbutes_t {
            log::attributes_t global;
            boost::thread_specific_ptr<scoped_attributes_concept_t> scoped;

            attrbutes_t(void(*deleter)(scoped_attributes_concept_t*)) :
                scoped(deleter)
            {}
        } attributes;

        std::vector<std::unique_ptr<base_frontend_t>> frontends;
        log::exception_handler_t exception;

        struct {
            mutable rw_mutex_type open;
            mutable rw_mutex_type push;
        } lock;

        state_t();
    };
    state_t state;

    friend class scoped_attributes_concept_t;

    friend void swap(logger_base_t& lhs, logger_base_t& rhs) BLACKHOLE_NOEXCEPT;

public:
    logger_base_t();

    //! @compat GCC4.4
    //! Blaming GCC4.4 - it needs explicit move constructor definition,
    //! because it cannot define default move constructor for derived class.
    logger_base_t(logger_base_t&& other) BLACKHOLE_NOEXCEPT;
    logger_base_t& operator=(logger_base_t&& other) BLACKHOLE_NOEXCEPT;

    bool enabled() const;
    void enabled(bool enable);

    bool tracked() const;
    void tracked(bool enable);

    void set_filter(filter_t&& filter);
    void add_attribute(const log::attribute_pair_t& attribute);
    void add_frontend(std::unique_ptr<base_frontend_t> frontend);
    void set_exception_handler(log::exception_handler_t&& handler);

    log::record_t open_record() const;
    log::record_t open_record(log::attribute_pair_t local_attribute) const;
    log::record_t open_record(log::attributes_t local_attributes) const;

    void push(log::record_t&& record) const;

private:
    log::attributes_t get_event_attributes() const;
    log::attributes_t get_thread_attributes() const;
};

//!@todo: Develop ImmutableLogger class, which provides almost immutable
//!       internal state (filter, exception handler, frontends).
//!       This class won't require any synchronization mechanizm.

// NOTE: It's not movable to avoid moving to another thread.
class scoped_attributes_concept_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(scoped_attributes_concept_t);

    logger_base_t *m_logger;
    scoped_attributes_concept_t *m_previous;

    friend void swap(logger_base_t&, logger_base_t&) BLACKHOLE_NOEXCEPT;

public:
    scoped_attributes_concept_t(logger_base_t& log);
    virtual ~scoped_attributes_concept_t();

    virtual const log::attributes_t& attributes() const = 0;

protected:
    bool has_parent() const;
    const scoped_attributes_concept_t& parent() const;
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
public:
    typedef Level level_type;

private:
    level_type level;

public:
    verbose_logger_t() :
        logger_base_t(),
        level(static_cast<level_type>(0))
    {}

    //! @compat GCC4.4
    //! GCC 4.4 doesn't create default copy/move constructor for derived
    //! classes. It's a bug.
    verbose_logger_t(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT :
        logger_base_t(std::move(other)),
        level(static_cast<level_type>(other.level))
    {}

    verbose_logger_t& operator=(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT {
        logger_base_t::operator=(std::move(other));
        level = other.level;
        return *this;
    }

    // Explicit import other overloaded methods.
    using logger_base_t::open_record;

    /*!
     * Gets the current upper verbosity bound.
     */
    level_type verbosity() const {
        return level;
    }

    /*!
     * Sets the upper verbosity bound.
     * Every log event with a verbosity less than `level` will be dropped.
     * @param[in] level - Upper verbosity value.
     */
    void verbosity(level_type level) {
        this->level = level;
    }

    /*! @todo: Documentation is @deprecated.
     * Tries to open log record with specific verbosity level.
     * Internally this method compares desired verbosity level with the upper
     * one. Can return invalid log record if some conditions are not met.
     * @param[in] level - Desired verbosity level.
     * @return valid or invalid `log::record_t` object.
     */
    log::record_t
    open_record(level_type level,
                log::attributes_t local = log::attributes_t()) const
    {
        typedef logger_verbosity_traits<level_type> verbosity_traits;
        const bool passed = verbosity_traits::passed(this->level, level);

        bool trace = false;
        if (!passed) {
            auto it = local.find(keyword::tracebit().name());
            if (it != local.end()) {
                trace = boost::get<keyword::tag::tracebit_t::type>(it->second.value);
            } else {
                reader_lock_type lock(state.lock.open);
                if (state.attributes.scoped.get()) {
                    const auto& scoped = state.attributes.scoped->attributes();
                    auto scoped_it = scoped.find(keyword::tracebit().name());
                    if (scoped_it != scoped.end()) {
                        trace = boost::get<keyword::tag::tracebit_t::type>(
                            scoped_it->second.value
                        );
                    }
                }
            }
        }

        if (passed || trace) {
            log::attributes_t attributes = { keyword::severity<Level>() = level };
            attributes.insert(local.begin(), local.end());
            return logger_base_t::open_record(std::move(attributes));
        }

        return log::record_t();
    }
};

} // namespace blackhole

#if defined(BLACKHOLE_HEADER_ONLY)
#include "blackhole/implementation/logger.ipp"
#endif
