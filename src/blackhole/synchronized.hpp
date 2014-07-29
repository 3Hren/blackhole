#pragma once

#include <mutex>

#include <boost/thread.hpp>

#include "blackhole/forwards.hpp"
#include "blackhole/logger.hpp"

namespace blackhole {

/*!
 * Wraps the logger, providing the same interface and making it thread-safe.
 *
 * This is achieved by using stupid mutex, locking it just before calling every
 * underlying logger's method. Not very super-fast, indeed.
 *
 * @deprecated: This class is deprecated and will be removed at 0.2.
 *              Logger itself will become thread-safe when compiling and linking
 *              with pthreads.
 */
template<class Logger, class Mutex>
class synchronized {
public:
    typedef Logger logger_type;
    typedef Mutex mutex_type;

private:
    logger_type log_;
    mutable mutex_type mutex;

    friend class scoped_attributes_t;

public:
    synchronized() {}

    explicit synchronized(logger_type&& logger) :
        log_(std::move(logger))
    {}

    synchronized(synchronized&& other) {
        *this = std::move(other);
    }

    synchronized& operator=(synchronized&& other) {
        //! @compat GCC4.4
        //! GCC4.4 doesn't implement `std::lock` as like as `std::adopt_lock`.
        boost::lock(mutex, other.mutex);
        boost::lock_guard<mutex_type> lock(mutex, boost::adopt_lock);
        boost::lock_guard<mutex_type> other_lock(other.mutex, boost::adopt_lock);
        log_ = std::move(other.log_);
        return *this;
    }

    logger_type& log() {
        return log_;
    }

    bool enabled() const {
        std::lock_guard<mutex_type> lock(mutex);
        return log_.enabled();
    }

    void enable() {
        std::lock_guard<mutex_type> lock(mutex);
        log_.enable();
    }

    void disable() {
        std::lock_guard<mutex_type> lock(mutex);
        log_.disable();
    }

    void set_filter(filter_t&& filter) {
        std::lock_guard<mutex_type> lock(mutex);
        log_.set_filter(std::move(filter));
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        std::lock_guard<mutex_type> lock(mutex);
        log_.add_attribute(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        std::lock_guard<mutex_type> lock(mutex);
        log_.add_frontend(std::move(frontend));
    }

    void set_exception_handler(log::exception_handler_t&& handler) {
        std::lock_guard<mutex_type> lock(mutex);
        log_.set_exception_handler(std::move(handler));
    }

    template<typename... Args>
    log::record_t open_record(Args&&... args) const {
        std::lock_guard<mutex_type> lock(mutex);
        return log_.open_record(std::forward<Args>(args)...);
    }

    void push(log::record_t&& record) const {
        std::lock_guard<mutex_type> lock(mutex);
        log_.push(std::move(record));
    }

    template<typename Level>
    typename std::enable_if<
        std::is_same<
            logger_type,
            verbose_logger_t<Level>
        >::value,
        Level
    >::type
    verbosity() const {
        std::lock_guard<mutex_type> lock(mutex);
        return log_.verbosity();
    }

    template<typename Level>
    typename std::enable_if<
        std::is_same<
            logger_type,
            verbose_logger_t<Level>
        >::value
    >::type
    verbosity(Level level) {
        std::lock_guard<mutex_type> lock(mutex);
        return log_.verbosity(level);
    }
};

} // namespace blackhole
