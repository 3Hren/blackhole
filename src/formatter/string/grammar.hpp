#pragma once

#include "token.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {
namespace string {

auto parse_leftover(const std::string& pattern) -> ph::leftover_t;
auto parse_optional(const std::string& name, const std::string& pattern) -> ph::generic<optional>;

} // namespace string
} // namespace formatter
} // namespace v1
} // namespace blackhole
