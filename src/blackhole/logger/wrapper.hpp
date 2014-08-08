#pragma once

#include "blackhole/config.hpp"
#include "blackhole/detail/thread/lock.hpp"
#include "blackhole/forwards.hpp"
#include "blackhole/record.hpp"
#include "blackhole/utils/noncopyable.hpp"
#include "blackhole/config.hpp"

namespace blackhole {

template<class Wrapper>
struct unwrap {
    typedef Wrapper wrapper_type;

    typedef typename unwrap<
        typename wrapper_type::underlying_type
    >::logger_type logger_type;

    static logger_type& log(Wrapper& wrapper) {
        return unwrap<
            typename wrapper_type::underlying_type
        >::log(*wrapper.wrapped);
    }
};

template<>
struct unwrap<logger_base_t> {
    typedef logger_base_t wrapper_type;
    typedef logger_base_t logger_type;

    static logger_type& log(logger_type& log) {
        return log;
    }
};

template<typename Level>
struct unwrap<verbose_logger_t<Level>> {
    typedef verbose_logger_t<Level> wrapper_type;
    typedef verbose_logger_t<Level> logger_type;

    static logger_type& log(logger_type& log) {
        return log;
    }
};

template<class Wrapped>
class wrapper_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(wrapper_t);

    template<class> friend struct unwrap;

public:
    typedef Wrapped underlying_type;
    typedef typename unwrap<underlying_type>::logger_type logger_type;

private:
    underlying_type* wrapped;
    log::attributes_t attributes;

    mutable std::mutex mutex;

public:
    wrapper_t(underlying_type& wrapped, log::attributes_t attributes) :
        wrapped(&wrapped),
        attributes(std::move(attributes))
    {}

    wrapper_t(const wrapper_t& wrapper, log::attributes_t attributes) :
        wrapped(wrapper.wrapped),
        attributes(merge({ wrapper.attributes, std::move(attributes) }))
    {}

    wrapper_t(wrapper_t&& other) {
        *this = std::move(other);
    }

    wrapper_t& operator=(wrapper_t&& other) {
        if (this != &other) {
            auto lock = detail::thread::make_multi_lock_t(mutex, other.mutex);
            wrapped = other.wrapped;
            other.wrapped = nullptr;
            attributes = std::move(other.attributes);
        }

        return *this;
    }

    /*!
     * Return non-const reference to the underlying logger.
     */
    logger_type& log() {
        return unwrap<wrapper_t>::log(*this);
    }

    log::record_t open_record() const {
        return wrapped->open_record(attributes);
    }

    log::record_t open_record(log::attribute_pair_t attribute) const {
        log::attributes_t attributes = this->attributes;
        attributes.insert(attribute);
        return wrapped->open_record(std::move(attributes));
    }

    log::record_t open_record(log::attributes_t attributes) const {
        log::attributes_t attributes_ = this->attributes;
        attributes_.insert(attributes.begin(), attributes.end());
        return wrapped->open_record(std::move(attributes_));
    }

    //!@todo: Add more gentle concept check.
    template<typename Level>
    log::record_t
    open_record(Level level,
                log::attributes_t attributes = log::attributes_t()) const
    {
        attributes.insert(this->attributes.begin(), this->attributes.end());
        return wrapped->open_record(level, std::move(attributes));
    }

    void push(log::record_t&& record) const {
        wrapped->push(std::move(record));
    }
};

} // namespace blackhole
