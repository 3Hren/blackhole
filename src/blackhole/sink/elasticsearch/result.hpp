#pragma once

#include <boost/system/error_code.hpp>
#include <boost/variant.hpp>

namespace elasticsearch {

template<class T>
struct result_t : public boost::variant<T, boost::system::error_code> {};

template<>
struct result_t<void> : public boost::variant<boost::system::error_code> {};

} // namespace elasticsearch
