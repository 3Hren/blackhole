#pragma once

#include <boost/variant/variant.hpp>

namespace blackhole {
inline namespace v1 {
namespace attribute {

struct value_t::inner_t {
    typedef boost::make_variant_over<value_t::types>::type type;

    type value;

    template<typename T>
    inner_t(T&& value) : value(std::forward<T>(value)) {}
};

struct view_t::inner_t {
    typedef boost::make_variant_over<view_t::types>::type type;

    type value;

    template<typename T>
    inner_t(T&& value) : value(std::forward<T>(value)) {}
};

}  // namespace attribute
}  // namespace v1
}  // namespace blackhole
