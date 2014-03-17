#pragma once

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/same.hpp"

namespace blackhole {

namespace aux {

template<class... Args>
struct is_keyword_pack : public are_same<log::attribute_pair_t, Args...> {};

} // namespace aux

} // namespace blackhole
