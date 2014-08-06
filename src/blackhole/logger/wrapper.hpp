#pragma once

#include "blackhole/config.hpp"
#include "blackhole/forwards.hpp"
#include "blackhole/record.hpp"
#include "blackhole/utils/noncopyable.hpp"
#include "blackhole/config.hpp"

namespace blackhole {

template<class Wrapper>
struct unwrap {
    typedef typename unwrap<
        typename Wrapper::underlying_type
    >::logger_type logger_type;
};

template<>
struct unwrap<logger_base_t> {
    typedef logger_base_t logger_type;
};

template<typename Level>
struct unwrap<verbose_logger_t<Level>> {
    typedef verbose_logger_t<Level> logger_type;
};

template<class Wrapped>
class wrapper_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(wrapper_t);

public:
    typedef Wrapped underlying_type;
    typedef typename unwrap<underlying_type>::logger_type logger_type;

private:
    underlying_type* log_;
    log::attributes_t attributes;

    mutable std::mutex mutex;

public:
    wrapper_t(underlying_type& log, log::attributes_t attributes) :
        log_(&log),
        attributes(std::move(attributes))
    {}

    wrapper_t(const wrapper_t& wrapper, log::attributes_t attributes) :
        log_(wrapper.log_),
        attributes(merge({ wrapper.attributes, std::move(attributes) }))
    {}

    wrapper_t(wrapper_t&& other) {
        *this = std::move(other);
    }

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

    underlying_type& log() {
        return *log_;
    }

    log::record_t open_record() const {
        return log_->open_record(attributes);
    }

    log::record_t open_record(log::attribute_pair_t attribute) const {
        log::attributes_t attributes = this->attributes;
        attributes.insert(attribute);
        return log_->open_record(std::move(attributes));
    }

    log::record_t open_record(log::attributes_t attributes) const {
        log::attributes_t attributes_ = this->attributes;
        attributes_.insert(attributes.begin(), attributes.end());
        return log_->open_record(std::move(attributes_));
    }

    //!@todo: Add more gentle concept check.
    template<typename Level>
    log::record_t
    open_record(Level level,
                log::attributes_t attributes = log::attributes_t()) const
    {
        attributes.insert(this->attributes.begin(), this->attributes.end());
        return log_->open_record(level, std::move(attributes));
    }

    void push(log::record_t&& record) const {
        log_->push(std::move(record));
    }
};

} // namespace blackhole
