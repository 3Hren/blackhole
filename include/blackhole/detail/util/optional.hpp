#pragma once

#include <boost/optional/optional_fwd.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace util {

/// This hack is required because of `boost::optional` 1.46, which I have to support and which
/// can not map on none value.
template<typename T, typename F>
auto value_or(const boost::optional<T>& optional, F fn) -> T {
    if (optional) {
        return optional.get();
    } else {
        return fn();
    }
}

}  // namespace util
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
