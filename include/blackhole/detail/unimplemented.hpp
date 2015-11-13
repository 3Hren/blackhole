#pragma once

#include <boost/assert.hpp>

#define BLACKHOLE_UNIMPLEMENTED() \
    BOOST_ASSERT(false && "not implemented yet")
