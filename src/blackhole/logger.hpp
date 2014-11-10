#pragma once

#include <memory>
#include <vector>

#include <boost/thread/tss.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/config/atomic.hpp"
#include "blackhole/detail/config/noncopyable.hpp"
#include "blackhole/detail/config/nullptr.hpp"
#include "blackhole/detail/util/unique.hpp"
#include "error/handler.hpp"
#include "filter.hpp"
#include "frontend.hpp"
#include "keyword.hpp"
#include "keyword/message.hpp"
#include "keyword/severity.hpp"
#include "keyword/thread.hpp"
#include "keyword/timestamp.hpp"
#include "keyword/tracebit.hpp"

#include "blackhole/config.hpp"

namespace blackhole {

class scoped_attributes_concept_t;

class logger_base_t {
    friend class scoped_attributes_concept_t;
    friend void swap(logger_base_t& lhs, logger_base_t& rhs) BLACKHOLE_NOEXCEPT;

protected:
    typedef boost::shared_mutex rw_mutex_type;
    typedef boost::shared_lock<rw_mutex_type> reader_lock_type;
    typedef boost::unique_lock<rw_mutex_type> writer_lock_type;

    struct state_t {
        std::atomic<bool> enabled;
        std::atomic<bool> tracked;

        filter_t filter;
        boost::thread_specific_ptr<scoped_attributes_concept_t> scoped;

        std::vector<std::unique_ptr<base_frontend_t>> frontends;
        log::exception_handler_t exception;

        struct {
            mutable rw_mutex_type open;
            mutable rw_mutex_type push;
        } lock;

        state_t();
    };

    state_t state;

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
    void add_frontend(std::unique_ptr<base_frontend_t> frontend);
    void set_exception_handler(log::exception_handler_t&& handler);

    record_t open_record() const;
    record_t open_record(attribute::pair_t attribute) const;
    record_t open_record(attribute::set_t attributes) const;

    void push(record_t&& record) const;

protected:
    record_t open_record(attribute::set_t internal, attribute::set_t external) const;
};

/// Concept form scoped attributes holder.
/*!
 * @note: It's not movable to avoid moving to another thread.
 */
class scoped_attributes_concept_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(scoped_attributes_concept_t);

    logger_base_t *m_logger;
    scoped_attributes_concept_t *m_previous;

    friend void swap(logger_base_t&, logger_base_t&) BLACKHOLE_NOEXCEPT;

public:
    scoped_attributes_concept_t(logger_base_t& log);
    virtual ~scoped_attributes_concept_t();

    virtual const attribute::set_t& attributes() const = 0;

protected:
    bool has_parent() const;
    const scoped_attributes_concept_t& parent() const;
};

template<typename Level>
class verbose_logger_t : public logger_base_t {
public:
    typedef Level level_type;

    typedef std::function<
        bool(level_type, const attribute::combined_view_t&)
    > filter_type;

private:
    level_type level;
    filter_type filter;

    // TODO: Thread-safety.

public:
    explicit verbose_logger_t(level_type level) :
        logger_base_t(),
        level(level),
        filter(default_filter { level })
    {}

    //! @compat: GCC4.4
    //! GCC 4.4 doesn't create default copy/move constructor for derived
    //! classes. It's a bug.
    verbose_logger_t(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT :
        logger_base_t(std::move(other))
    {
        level = other.level;
        filter = other.filter;
    }

    verbose_logger_t& operator=(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT {
        logger_base_t::operator=(std::move(other));

        level = other.level;
        filter = other.filter;
        return *this;
    }

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
        verbosity(level, default_filter { level });
    }

    // TODO: Maybe rename this to `filter` and the upper one?
    void verbosity(level_type level, filter_type filter) {
        this->level = level;
        this->filter = filter;
    }

    /*!
     * Tries to open log record with specific verbosity level.
     *
     * Internally this method compares desired verbosity level with the upper
     * one and checks for tracebit attribute (temporary until filter redesign).
     * Can return invalid log record if some conditions are not met.
     * @param[in] level - Desired verbosity level.
     * @return valid or invalid `record_t` object.
     * @todo: Decompose.
     */
    record_t
    open_record(level_type level, attribute::set_t local = attribute::set_t()) const {
        bool passed = false;
        reader_lock_type lock(state.lock.open);
        if (auto scoped = state.scoped.get()) {
            const attribute::combined_view_t view(local, scoped->attributes());
            passed = filter(level, view);
        } else {
            const attribute::combined_view_t view(local);
            passed = filter(level, view);
        }

        if (passed) {
            attribute::set_t internal;
            internal.emplace_back(keyword::severity<Level>() = level);
            return logger_base_t::open_record(std::move(internal), std::move(local));
        }

        return record_t();
    }

private:
    struct default_filter {
        level_type threshold;

        inline
        bool
        operator()(level_type level, const attribute::combined_view_t&) const {
            typedef typename aux::underlying_type<Level>::type underlying_type;

            return static_cast<underlying_type>(level) >= static_cast<underlying_type>(threshold);
        }
    };
};

} // namespace blackhole

#if defined(BLACKHOLE_HEADER_ONLY)
#include "blackhole/logger.ipp"
#endif
