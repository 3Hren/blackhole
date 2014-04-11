#pragma once

#include "blackhole/logger.hpp"

namespace blackhole {

template<class Logger>
class synchronized {
    Logger logger;
    mutable std::mutex mutex;
public:
    synchronized(Logger&& logger) :
        logger(std::move(logger))
    {}

    bool enabled() const {
        std::lock_guard<std::mutex> lock(mutex);
        return logger.enabled();
    }

    void enable() {
        std::lock_guard<std::mutex> lock(mutex);
        logger.enable();
    }

    void disable() {
        std::lock_guard<std::mutex> lock(mutex);
        logger.disable();
    }

    void set_filter(filter_t&& filter) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.set_filter(std::move(filter));
    }

    void add_attribute(const log::attribute_pair_t& attr) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.add_attribute(attr);
    }

    void add_frontend(std::unique_ptr<base_frontend_t> frontend) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.add_frontend(std::move(frontend));
    }

    void set_exception_handler(log::exception_handler_t&& handler) {
        std::lock_guard<std::mutex> lock(mutex);
        logger.set_exception_handler(std::move(handler));
    }

    template<typename... Args>
    log::record_t open_record(Args&&... args) const {
        std::lock_guard<std::mutex> lock(mutex);
        return logger.open_record(std::forward<Args>(args)...);
    }

    void push(log::record_t&& record) const {
        std::lock_guard<std::mutex> lock(mutex);
        logger.push(std::move(record));
    }
};

} // namespace blackhole
