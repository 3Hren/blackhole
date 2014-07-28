#pragma once

#include <boost/assert.hpp>

#include "blackhole/logger.hpp"

namespace blackhole {

template<class Logger>
wrapper_t<Logger>::wrapper_t(logger_type& log, log::attributes_t attributes) :
    log_(&log),
    attributes(std::move(attributes))
{}

template<class Logger>
wrapper_t<Logger>::wrapper_t(wrapper_t& wrapper, log::attributes_t attributes) :
    log_(wrapper.log_),
    attributes(merge({ wrapper.attributes, std::move(attributes) }))
{}

template<class Logger>
log::record_t
wrapper_t<Logger>::open_record() const {
    return log_->open_record(attributes);
}

template<class Logger>
log::record_t
wrapper_t<Logger>::open_record(log::attribute_pair_t attribute) const {
    log::attributes_t attributes = this->attributes;
    attributes.insert(attribute);
    return log_->open_record(std::move(attributes));
}

template<class Logger>
log::record_t
wrapper_t<Logger>::open_record(log::attributes_t attributes) const {
    log::attributes_t attributes_ = this->attributes;
    attributes_.insert(attributes.begin(), attributes.end());
    return log_->open_record(std::move(attributes_));
}

template<class Logger>
void
wrapper_t<Logger>::push(log::record_t&& record) const {
    log_->push(std::move(record));
}

} // namespace blackhole
