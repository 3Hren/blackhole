#pragma once

#include <ostream>
#include <sstream>
#include <string>

#include <boost/format.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/format/message/insitu.hpp"
#include "blackhole/keyword.hpp"

namespace blackhole {

namespace aux {

// Specialization of utils::format function for extracting and replacing `in situ` keyword arguments.
// For example actual level attribute string representation is substituted:
// BH_LOG(log, level::debug, "level = %s", keyword::severity_t<level>());

//!@todo: Consider migrating to something more faster than boost::format.
static inline
std::string
substitute(const log::attributes_t&, boost::format&& message) {
    return message.str();
}

template<typename T, typename... Args>
static inline
std::string
substitute(const log::attributes_t& attributes, boost::format&& message, const T& argument, const Args&... args) {
    return substitute(attributes, std::move(message % argument), args...);
}

template<typename T, typename Tag, log::attribute::scope Scope, typename... Args>
static inline
std::string
substitute(const log::attributes_t& attributes, boost::format&& message, const keyword::keyword_t<T, Tag, Scope>&, const Args&... args) {
    const T& arg = attribute::traits<T>::extract(attributes, keyword::keyword_t<T, Tag, Scope>::name());
    std::ostringstream stream;
    format::message::insitu<Tag>::execute(stream, arg);
    return substitute(attributes, std::move(message % stream.str()), args...);
}

template<typename... Args>
static std::string format(const log::attributes_t& attributes, const std::string& fmt, Args&&... args) {
    return substitute(attributes, boost::format(fmt), args...);
}

} // namespace aux

} // namespace blackhole
