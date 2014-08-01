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
#include "blackhole/utils/noexcept.hpp"
#include "utils/noncopyable.hpp"
#include "utils/unique.hpp"

namespace blackhole {

class scoped_attributes_concept_t;

template <typename Level>
struct logger_verbosity_traits
{
    static bool passed(Level logger_verbosity, Level record_verbosity)
    {
        typedef typename aux::underlying_type<Level>::type underlying_type;

        return static_cast<underlying_type>(record_verbosity) >=
            static_cast<underlying_type>(logger_verbosity);
    }
};

class logger_base_t {
    bool m_enabled;
    bool tracked_;

protected:
    filter_t m_filter;
    log::exception_handler_t m_exception_handler;

    std::vector<std::unique_ptr<base_frontend_t>> m_frontends;

    log::attributes_t m_global_attributes;

    boost::thread_specific_ptr<scoped_attributes_concept_t> m_scoped_attributes;

    friend class scoped_attributes_concept_t;

    friend void swap(logger_base_t&, logger_base_t&) BLACKHOLE_NOEXCEPT;

public:
    logger_base_t();

    //! @compat GCC4.4
    //! Blaming GCC4.4 - it needs explicit move constructor definition,
    //! because it cannot define default move constructor for derived class.
    logger_base_t(logger_base_t&& other) BLACKHOLE_NOEXCEPT;
    logger_base_t& operator=(logger_base_t&& other) BLACKHOLE_NOEXCEPT;

    bool enabled() const;
    void enable();
    void disable();
    void set_filter(filter_t&& filter);
    void add_attribute(const log::attribute_pair_t& attribute);
    void add_frontend(std::unique_ptr<base_frontend_t> frontend);
    void set_exception_handler(log::exception_handler_t&& handler);
    void track(bool enable = true);

    log::record_t open_record() const;
    log::record_t open_record(log::attribute_pair_t local_attribute) const;
    log::record_t open_record(log::attributes_t local_attributes) const;

    void push(log::record_t&& record) const;

private:
    log::attributes_t get_event_attributes() const;
    log::attributes_t get_thread_attributes() const;
};

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
    Level level;

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
        level(static_cast<level_type>(0))
    {}

    verbose_logger_t& operator=(verbose_logger_t&& other) BLACKHOLE_NOEXCEPT {
        logger_base_t::operator=(std::move(other));
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
            } else if (m_scoped_attributes.get()) {
                const auto& scoped = m_scoped_attributes->attributes();
                auto scoped_it = scoped.find(keyword::tracebit().name());
                if (scoped_it != scoped.end()) {
                    trace = boost::get<keyword::tag::tracebit_t::type>(scoped_it->second.value);
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
