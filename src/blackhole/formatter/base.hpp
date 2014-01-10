#pragma once

#include <blackhole/formatter/map/value.hpp>

namespace blackhole {

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

} // namespace blackhole
