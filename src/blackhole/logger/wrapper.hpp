#pragma once

#include "blackhole/config.hpp"
#include "blackhole/detail/thread/lock.hpp"
#include "blackhole/detail/config/noncopyable.hpp"
#include "blackhole/forwards.hpp"
#include "blackhole/record.hpp"
#include "blackhole/config.hpp"

namespace blackhole {

template<class Wrapper>
struct unwrap {
    typedef Wrapper wrapper_type;

    typedef typename unwrap<
        typename wrapper_type::underlying_type
    >::logger_type logger_type;

    static logger_type& log(wrapper_type& wrapper) {
        return unwrap<
            typename wrapper_type::underlying_type
        >::log(*wrapper.wrapped);
    }

    static const logger_type& log(const wrapper_type& wrapper) {
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

    static const logger_type& log(const logger_type& log) {
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

    static const logger_type& log(const logger_type& log) {
        return log;
    }
};

template<class Wrapped>
class wrapper_base_t {
    BLACKHOLE_DECLARE_NONCOPYABLE(wrapper_base_t);

    template<class> friend struct unwrap;

public:
    typedef Wrapped underlying_type;
    typedef typename unwrap<underlying_type>::logger_type logger_type;

protected:
    underlying_type* wrapped;
    attribute::set_t attributes;

    // TODO: Class isn't completely thread-safe.
    mutable std::mutex mutex;

public:
    wrapper_base_t(underlying_type& wrapped, attribute::set_t attributes) :
        wrapped(&wrapped),
        attributes(std::move(attributes))
    {}

    wrapper_base_t(const wrapper_base_t& wrapper, const attribute::set_t& attributes) :
        wrapped(wrapper.wrapped),
        attributes(wrapper.attributes)
    {
        std::copy(attributes.begin(), attributes.end(), std::back_inserter(this->attributes));
    }

    wrapper_base_t(wrapper_base_t&& other) {
        *this = std::move(other);
    }

    // TODO: Is it REALLY need to be assignable? If not - I can easily drop thread synchronization.
    wrapper_base_t& operator=(wrapper_base_t&& other) {
        if (this != &other) {
            auto lock = detail::thread::multi_lock(mutex, other.mutex);
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
        return unwrap<wrapper_base_t>::log(*this);
    }

    /*!
     * Return const reference to the underlying logger.
     */
    const logger_type& log() const {
        return unwrap<wrapper_base_t>::log(*this);
    }

    void push(record_t&& record) const {
        wrapped->push(std::move(record));
    }
};

template<class Wrapped>
class wrapper_t<
    Wrapped,
    typename std::enable_if<std::is_same<typename unwrap<Wrapped>::logger_type, logger_base_t>::value>::type
> : public wrapper_base_t<Wrapped>
{
    typedef wrapper_base_t<Wrapped> base_type;

public:
    typedef typename base_type::underlying_type underlying_type;

public:
    wrapper_t(underlying_type& wrapped, attribute::set_t attributes) :
        base_type(wrapped, std::move(attributes))
    {}

    wrapper_t(const wrapper_t& wrapper, const attribute::set_t& attributes) :
        base_type(wrapper, attributes)
    {}

    wrapper_t(wrapper_t&& other) :
        base_type(std::move(other))
    {}

    wrapper_t& operator=(wrapper_t&& other) {
        base_type::operator =(std::move(other));
    }

    record_t open_record() const {
        return this->wrapped->open_record(this->attributes);
    }

    // TODO: Do this under lock or drop assignment.
    record_t open_record(attribute::pair_t attribute) const {
        attribute::set_t attributes = this->attributes;
        attributes.emplace_back(attribute);
        return this->wrapped->open_record(std::move(attributes));
    }

    // TODO: Do this under lock or drop assignment.
    record_t open_record(attribute::set_t attributes) const {
        std::copy(this->attributes.begin(), this->attributes.end(), std::back_inserter(attributes));
        return this->wrapped->open_record(std::move(attributes));
    }
};

template<class Wrapped>
class wrapper_t<
    Wrapped,
    typename std::enable_if<std::is_same<typename unwrap<Wrapped>::logger_type, verbose_logger_t<typename unwrap<Wrapped>::logger_type::level_type>>::value>::type
> : public wrapper_base_t<Wrapped>
{
    typedef wrapper_base_t<Wrapped> base_type;

public:
    typedef typename base_type::underlying_type underlying_type;

public:
    wrapper_t(underlying_type& wrapped, attribute::set_t attributes) :
        base_type(wrapped, std::move(attributes))
    {}

    wrapper_t(const wrapper_t& wrapper, const attribute::set_t& attributes) :
        base_type(wrapper, attributes)
    {}

    wrapper_t(wrapper_t&& other) :
        base_type(std::move(other))
    {}

    wrapper_t& operator=(wrapper_t&& other) {
        base_type::operator =(std::move(other));
        return *this;
    }

    template<typename Level>
    record_t
    open_record(Level level, attribute::set_t attributes = attribute::set_t()) const {
        // TODO: Do this under lock or drop assignment.
        std::copy(this->attributes.begin(), this->attributes.end(), std::back_inserter(attributes));
        return this->wrapped->open_record(level, std::move(attributes));
    }
};

} // namespace blackhole
