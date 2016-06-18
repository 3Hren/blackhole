#pragma once

#include "blackhole/stdext/string_view.hpp"
#include "blackhole/extensions/format.hpp"

namespace blackhole {
inline namespace v1 {

using stdext::string_view;

/// Represents stream writer backed up by cppformat.
class writer_t {
public:
    fmt::MemoryWriter inner;

    /// Formats the given arguments using the underlying formatter.
    template<typename... Args>
    inline auto write(const Args&... args) -> void {
        inner.write(args...);
    }

    auto result() const noexcept -> string_view {
        return string_view(inner.data(), inner.size());
    }
};

}  // namespace v1
}  // namespace blackhole
