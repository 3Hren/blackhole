#pragma once

#include <cppformat/format.h>

namespace blackhole {

/// Represents stream writer backed up by cppformat.
class writer_t {
public:
    fmt::MemoryWriter inner;

    /// Formats the given arguments using the underlying formatter.
    template<typename... Args>
    inline auto write(const Args&... args) -> void {
        inner.write(args...);
    }
};

}  // namespace blackhole
