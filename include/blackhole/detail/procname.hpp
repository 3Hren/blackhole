#pragma once

#include <string>

namespace blackhole {
inline namespace v1 {
namespace stdext {

template<typename Char, typename Traits> class basic_string_view;
typedef basic_string_view<char, std::char_traits<char>> string_view;

}  // namespace stdext

namespace detail {

auto procname() -> stdext::string_view;

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
