#pragma once

#include <cstddef>
#include <memory>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {

/// Flush suggest policy.
class flusher_t {
public:
    enum result_t {
        /// No flush required.
        idle,
        /// It's time to flush.
        flush
    };

public:
    virtual ~flusher_t() = default;

    /// Resets the current flusher state.
    virtual auto reset() -> void = 0;

    /// Updates the flusher, incrementing its counters.
    ///
    /// \param nwritten bytes consumed during previous write operation.
    virtual auto update(std::size_t nwritten) -> result_t = 0;
};

class flusher_factory_t {
public:
    virtual ~flusher_factory_t() = default;
    virtual auto create() const -> std::unique_ptr<flusher_t> = 0;
};

}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
