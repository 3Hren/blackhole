#pragma once

#include <type_traits>

#include <boost/system/error_code.hpp>
#include <boost/variant.hpp>

namespace elasticsearch {

template<class T>
struct result_t : public boost::variant<T, boost::system::error_code> {
    typedef boost::variant<T, boost::system::error_code> base_type;

    template<typename C>
    explicit result_t(C&& c) :
        base_type(std::forward<C>(c))
    {}
};

} // namespace elasticsearch
