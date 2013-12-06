#pragma once

#include "blackhole/keyword.hpp"

namespace blackhole {

namespace keyword {

namespace tag {

struct severity_t {
    static const char* name() { return "severity"; }
};

} // namespace tag

template<typename T>
static keyword_t<T, tag::severity_t, log::attribute_t::type_t::event>& severity() {
    static keyword_t<T, tag::severity_t, log::attribute_t::type_t::event> self;
    return self;
}

} // namespace keyword

} // namespace blackhole
