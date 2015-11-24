#include "blackhole/config/none.hpp"

#include "blackhole/config/monadic.hpp"

namespace blackhole {
namespace config {

auto
none_t::operator[](const std::size_t&) const -> monadic<config_t> {
    return {};
}

auto
none_t::operator[](const std::string&) const -> monadic<config_t> {
    return {};
}

auto
none_t::is_nil() const -> bool {
    return true;
}

auto
none_t::is_bool() const -> bool {
    return false;
}

auto
none_t::is_i64() const -> bool {
    return false;
}

auto
none_t::is_u64() const -> bool {
    return false;
}

auto
none_t::is_double() const -> bool {
    return false;
}

auto
none_t::is_string() const -> bool {
    return false;
}

auto
none_t::is_vector() const -> bool {
    return false;
}

auto
none_t::is_object() const -> bool {
    return false;
}

auto
none_t::to_bool() const -> bool {
    throw bad_optional_access();
}

auto
none_t::to_i64() const -> std::int64_t {
    throw bad_optional_access();
}

auto
none_t::to_u64() const -> std::uint64_t {
    throw bad_optional_access();
}

auto
none_t::to_double() const -> double {
    throw bad_optional_access();
}

auto
none_t::to_string() const -> std::string {
    throw bad_optional_access();
}

auto
none_t::each(const each_function&) -> void {
    throw bad_optional_access();
}

auto
none_t::each_map(const member_function&) -> void {
    throw bad_optional_access();
}

}  // namespace config
}  // namespace blackhole
