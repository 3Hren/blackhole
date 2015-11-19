#pragma once

#include <string>

namespace blackhole {
namespace cpp17 {

template<typename Char, typename Traits> class basic_string_view;
typedef basic_string_view<char, std::char_traits<char>> string_view;

}  // namespace cpp17

namespace detail {

auto procname() -> cpp17::string_view;

}  // namespace detail
}  // namespace blackhole
