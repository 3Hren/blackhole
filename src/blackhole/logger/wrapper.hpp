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
    logger_type& log_;
    const log::attributes_t attributes;

public:
    wrapper_t(logger_type& log, log::attributes_t attributes);

    logger_type& log() {
        return log_;
    }

    log::record_t open_record() const;
    log::record_t open_record(log::attribute_pair_t attribute) const;
    log::record_t open_record(log::attributes_t attributes) const;

    //!@todo: Add more gentle concept check.
    template<typename Level>
    log::record_t
    open_record(Level level) const {
        log::attributes_t attributes = this->attributes;
        attributes.insert(keyword::severity<Level>() = level);
        return log_.open_record(std::move(attributes));
    }

    void push(log::record_t&& record) const;
};

} // namespace blackhole

#if defined(BLACKHOLE_HEADER_ONLY)
#include "blackhole/implementation/logger/wrapper.ipp"
#endif
