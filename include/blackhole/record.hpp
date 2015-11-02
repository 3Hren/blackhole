#pragma once

#include <chrono>
#include <cstdint>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

using cpp17::string_view;

class record_t {
public:
    typedef std::chrono::high_resolution_clock clock_type;
    typedef clock_type::time_point time_point;

private:

public:
    auto message() const -> string_view;
    auto severity() const -> int;
    auto timestamp() const -> time_point;

    auto pid() const -> std::uint64_t;
    auto tid() const -> std::uint64_t;

    // auto attributes() const -> const range_type&;
};

}  // namespace blackhole
