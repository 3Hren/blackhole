#pragma once

#include <chrono>
#include <cstdint>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

using cpp17::string_view;

class record_t {
public:
    typedef std::chrono::high_resolution_clock clock_type;
    typedef clock_type::time_point time_point;

private:
    struct inner_t;

    typedef std::aligned_storage<64>::type storage_type;
    storage_type storage;

public:
    record_t(int severity, const string_view& message, const attribute_pack& attributes);

    auto message() const noexcept -> const string_view&;
    auto severity() const noexcept -> int;
    auto timestamp() const noexcept -> time_point;

    auto pid() const noexcept -> std::uint64_t;
    auto tid() const noexcept -> std::uint64_t;

    auto formatted() const noexcept -> const string_view&;
    auto attributes() const noexcept -> const attribute_pack&;

    auto activate(const string_view& formatted = string_view()) noexcept -> void;

private:
    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;
};

}  // namespace blackhole
