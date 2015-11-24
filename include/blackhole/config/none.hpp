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

    auto is_nil() const -> bool;
    auto is_bool() const -> bool;
    auto is_i64() const -> bool;
    auto is_u64() const -> bool;
    auto is_double() const -> bool;
    auto is_string() const -> bool;
    auto is_vector() const -> bool;
    auto is_object() const -> bool;

    auto to_bool() const -> bool;
    auto to_i64() const -> std::int64_t;
    auto to_u64() const -> std::uint64_t;
    auto to_double() const -> double;
    auto to_string() const -> std::string;

    auto each(const each_function& fn) -> void;
    auto each_map(const member_function& fn) -> void;
};

}  // namespace config
}  // namespace blackhole
