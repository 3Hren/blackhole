#pragma once

#include "blackhole/config.hpp"

#include "blackhole/formatter/map/value.hpp"

BLACKHOLE_BEG_NS

namespace formatter {

class base_t {
protected:
    mapping::value_t mapper;

public:
    void set_mapper(const mapping::value_t& mapper) {
        this->mapper = mapper;
    }

    void set_mapper(mapping::value_t&& mapper) {
        this->mapper = std::move(mapper);
    }
};

} // namespace formatter

BLACKHOLE_END_NS
