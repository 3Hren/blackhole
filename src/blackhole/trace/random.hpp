#pragma once

#include "distribution.hpp"

template<typename Distribution>
class random_t {
public:
    typedef Distribution distribution_type;
    typedef typename distribution_type::value_type value_type;

private:
    distribution_type distribution_;

public:
    static random_t& instance() {
        static random_t self;
        return self;
    }

    distribution_type& distribution() {
        return distribution_;
    }

    value_type next() {
        return distribution_.next();
    }

private:
    random_t() {}
};
