#pragma once

#include <limits>
#include <random>

#include "blackhole/platform/random.hpp"

#if !defined(BLACKHOLE_HAS_CXX11_RANDOM)
#include <time.h>
#endif

template<typename Value>
struct distribution_t {
    typedef Value value_type;

#if defined(BLACKHOLE_HAS_CXX11_RANDOM)
private:
    std::random_device device;
    std::mt19937 generator;
    std::uniform_int_distribution<value_type> distribution;

public:
    distribution_t() :
        generator(device()),
        distribution(min(), max())
    {}
#else
private:
    std::mt19937 generator;
    std::uniform_int<value_type> distribution;

public:
    distribution_t() :
        generator(static_cast<value_type>(std::time(nullptr))),
        distribution(min(), max())
    {}
#endif

    value_type next() {
        return distribution(generator);
    }

private:
    static value_type min() {
        return 1;
    }

    static value_type max() {
        return std::numeric_limits<value_type>::max();
    }
};
