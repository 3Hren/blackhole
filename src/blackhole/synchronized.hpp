#pragma once

#include <mutex>

#include <boost/thread.hpp>

#include "blackhole/logger.hpp"

namespace blackhole {

template<class Logger, class Mutex = std::mutex>
class synchronized {
public:
    typedef Logger logger_type;
    typedef Mutex mutex_type;

private:
    logger_type logger;
    mutable mutex_type mutex;

public:
    synchronized() {}

    explicit synchronized(logger_type&& logger) :
        logger(std::move(logger))
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
        log = std::move(other.log);
        return *this;
    }

    bool enabled() const {
        std::lock_guard<mutex_type> lock(mutex);
        return logger.enabled();
    }

    void enable() {
        std::lock_guard<mutex_type> lock(mutex);
        logger.enable();
    }

    void disable() {
        std::lock_guard<mutex_type> lock(mutex);
        logger.disable();
    }

    void set_filter(filter_t&& filter) {
        std::lock_guard<mutex_type> lock(mutex);
        logger.set_filter(std::move(filter));
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        std::lock_guard<mutex_type> lock(mutex);
        logger.add_attribute(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        std::lock_guard<mutex_type> lock(mutex);
        logger.add_frontend(std::move(frontend));
    }

    void set_exception_handler(log::exception_handler_t&& handler) {
        std::lock_guard<mutex_type> lock(mutex);
        logger.set_exception_handler(std::move(handler));
    }

    template<typename... Args>
    log::record_t open_record(Args&&... args) const {
        std::lock_guard<mutex_type> lock(mutex);
        return logger.open_record(std::forward<Args>(args)...);
    }

    void push(log::record_t&& record) const {
        std::lock_guard<mutex_type> lock(mutex);
        logger.push(std::move(record));
    }
};

} // namespace blackhole
