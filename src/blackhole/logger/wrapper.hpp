#pragma once

#include "blackhole/record.hpp"
#include "blackhole/utils/noncopyable.hpp"

namespace blackhole {

template<class Logger>
class wrapper_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(wrapper_t<Logger>);

public:
    typedef Logger logger_type;

private:
    logger_type* log_;
    log::attributes_t attributes;

    mutable std::mutex mutex;

public:
    wrapper_t(logger_type& log, log::attributes_t attributes);
    wrapper_t(wrapper_t& wrapper, log::attributes_t attributes);

    //!@todo: Test.
    wrapper_t(wrapper_t&& other) {
        std::lock_guard<std::mutex> lock(mutex);
        *this = std::move(other);
    }

    //!@todo: Test.
    wrapper_t& operator=(wrapper_t&& other) {
        if (this != &other) {
            boost::lock(mutex, other.mutex);
            boost::lock_guard<std::mutex> lock(mutex, boost::adopt_lock);
            boost::lock_guard<std::mutex> other_lock(other.mutex, boost::adopt_lock);
            log_ = other.log_;
            other.log_ = nullptr;
            attributes = std::move(other.attributes);
        }

        return *this;
    }

    logger_type& log() {
        return *log_;
    }

    log::record_t open_record() const;
    log::record_t open_record(log::attribute_pair_t attribute) const;
    log::record_t open_record(log::attributes_t attributes) const;

    //!@todo: Add more gentle concept check.
    template<typename Level>
    log::record_t
    open_record(Level level,
                log::attributes_t attributes = log::attributes_t()) const
    {
        attributes.insert(this->attributes.begin(), this->attributes.end());
        return log_->open_record(level, std::move(attributes));
    }

    void push(log::record_t&& record) const;
};

} // namespace blackhole

#if defined(BLACKHOLE_HEADER_ONLY)
#include "blackhole/implementation/logger/wrapper.ipp"
#endif
