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
