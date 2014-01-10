#pragma once

#include "blackhole/keyword.hpp"

namespace blackhole {

namespace keyword {

namespace tag {

template<typename T>
struct severity_t {
    typedef T type;
    static const char* name() { return "severity"; }
};

} // namespace tag

template<typename T>
static keyword_t<T, tag::severity_t<T>, log::attribute::scope::event>& severity() {
    static keyword_t<T, tag::severity_t<T>, log::attribute::scope::event> self;
    return self;
}

} // namespace keyword

} // namespace blackhole
