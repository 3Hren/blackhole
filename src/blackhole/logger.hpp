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
#include "keyword/timestamp.hpp"
#include "keyword/thread.hpp"
#include "universe.hpp"
#include "blackhole/utils/noexcept.hpp"
#include "utils/noncopyable.hpp"
#include "utils/unique.hpp"

namespace blackhole {

class scoped_attributes_concept_t;

class logger_base_t {
    bool m_enabled;

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

    log::record_t open_record(level_type level) const {
        typedef typename aux::underlying_type<level_type>::type underlying_type;
        if (static_cast<underlying_type>(level) < static_cast<underlying_type>(this->level)) {
            return log::record_t();
        }

        log::attributes_t attributes = { keyword::severity<Level>() = level };
        return logger_base_t::open_record(std::move(attributes));
    }
};

} // namespace blackhole

#if defined(BLACKHOLE_HEADER_ONLY)
#include "blackhole/implementation/logger.ipp"
#endif
