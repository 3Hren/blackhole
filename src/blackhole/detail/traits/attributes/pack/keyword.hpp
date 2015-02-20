#pragma once

#include "blackhole/config.hpp"

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/same.hpp"

BLACKHOLE_BEG_NS

namespace aux {

template<class... Args>
struct is_keyword_pack : public are_same<attribute::pair_t, Args...> {};

} // namespace aux

BLACKHOLE_END_NS
