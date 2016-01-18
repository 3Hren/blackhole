#pragma once

namespace blackhole {
inline namespace v1 {

class record_t;

class writer_t;

/// Represents an interface that every formatter must implement.
///
/// Formatters are responsible for formatting the input logging record using the specified writer.
class formatter_t {
public:
    formatter_t() = default;
    formatter_t(const formatter_t& other) = default;
    formatter_t(formatter_t&& other) = default;

    virtual ~formatter_t() = 0;

    /// Formats the specified logging event record by invoking formatter renderers and writing the
    /// result into the given writer.
    virtual auto format(const record_t& record, writer_t& writer) -> void = 0;
};

}  // namespace v1
}  // namespace blackhole
