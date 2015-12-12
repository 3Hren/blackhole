#pragma once

#include "blackhole/config.hpp"

namespace blackhole {
namespace config {

template<typename>
class monadic;

/// None value. Throws an exception on any get access, but maps to none on subscription.
class none_t : public config_t {
public:
    auto operator[](const std::size_t& idx) const -> monadic<config_t>;
    auto operator[](const std::string& key) const -> monadic<config_t>;

    auto to_bool() const -> bool;
    auto to_int64() const -> std::int64_t;
    auto to_uint64() const -> std::uint64_t;
    auto to_double() const -> double;
    auto to_string() const -> std::string;

    auto each(const each_function& fn) -> void;
    auto each_map(const member_function& fn) -> void;
};

}  // namespace config
}  // namespace blackhole
