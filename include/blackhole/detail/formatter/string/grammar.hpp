#pragma once

#include <blackhole/detail/formatter/string/token.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {

auto parse_leftover(const std::string& pattern) -> ph::leftover_t;

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
