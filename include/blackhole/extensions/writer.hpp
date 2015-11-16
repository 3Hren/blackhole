#pragma once

#include <cppformat/format.h>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

using cpp17::string_view;

/// Represents stream writer backed up by cppformat.
class writer_t {
public:
    fmt::MemoryWriter inner;

    /// Formats the given arguments using the underlying formatter.
    template<typename... Args>
    inline auto write(const Args&... args) -> void {
        inner.write(args...);
    }

    constexpr auto result() const noexcept -> string_view {
        return string_view(inner.data(), inner.size());
    }
};

}  // namespace blackhole
