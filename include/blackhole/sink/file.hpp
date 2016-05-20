#pragma once

#include <memory>

#include "blackhole/factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

/// Represents a sink that writes formatted log events to the file or files located at the specified
/// path.
///
/// The path can contain attribute placeholders, meaning that the real destination name will be
/// deduced at runtime using provided log record. No real file will be opened at construction
/// time.
/// All files are opened by default in append mode meaning seek to the end of stream immediately
/// after open.
///
/// \note associated files will be opened on demand during the first write operation.
class file_t;

}  // namespace sink

namespace experimental {

/// Represents a file sink builder to ease its configuration.
template<>
class builder<sink::file_t> {
public:
    struct bytes_t {
        std::uint64_t value;
    };

private:
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> p;

public:
    explicit builder(const std::string& path);

    /// Specifies a flush threshold in terms of bytes written.
    ///
    /// Logging backend will flush its internal buffers after at least every count bytes written,
    /// but the underlying implementation can decide to do it more often. Note that 0 value means
    /// automatic policy.
    ///
    /// \param count flush threshold.
    auto threshold(bytes_t count) -> builder&;

    /// Specifies a flush threshold in terms of write operations.
    ///
    /// Logging backend will flush its internal buffers after at least every count writes, but the
    /// underlying implementation can decide to do it more often. Note that 0 value means automatic
    /// policy.
    ///
    /// \param count flush threshold.
    auto threshold(std::size_t count) -> builder&;

    /// Consumes this builder returning a file sink.
    auto build() && -> std::unique_ptr<sink_t>;
};

template<>
class factory<sink::file_t> : public factory<sink_t> {
public:
    auto type() const noexcept -> const char*;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t>;
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
