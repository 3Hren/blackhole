#pragma once

#include <string>
#include <typeindex>

#include "blackhole/dynamic.hpp"

namespace blackhole {

template<typename T>
struct match_traits {
    static std::type_index ti(const std::string&, const dynamic_t&) {
        return typeid(T);
    }
};

} // namespace blackhole
