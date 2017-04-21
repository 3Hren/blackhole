#pragma once

#include <boost/assert.hpp>

/// Three ways of ignorance.
#define BLACKHOLE_UNIMPLEMENTED() \
    BOOST_ASSERT(false && "not implemented yet"); \
    throw std::runtime_error("not implemented yet"); \
    std::terminate()
