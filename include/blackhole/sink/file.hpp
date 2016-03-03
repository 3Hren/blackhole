#pragma once

#include "blackhole/sink.hpp"

#include <memory>

namespace blackhole {
inline namespace v1 {

template<typename>
struct factory;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace sink {

namespace file {

class inner_t;

}  // namespace file

class file_t : public sink_t {
    class properties_t;

    std::unique_ptr<file::inner_t> inner;

public:
    /// Represents a file sink object builder to ease its configuration.
    class builder_t;

    /// Constructs a file sink, which will write all incoming events to the file or files located at
    /// the specified path.
    ///
    /// The path can contain attribute placeholders, meaning that the real destination name will be
    /// deduced at runtime using provided log record. No real file will be opened at construction
    /// time.
    /// The file is opened by default in append mode meaning seek to the end of stream immediately
    /// after open.
    ///
    /// \param filename actually a path with final destination file to open. All files are opened
    ///     with append mode.
    /// \note the associated files will be opened just right after the first write operation.
    explicit file_t(const std::string& filename);

    /// Copy construction is explicitly prohibited.
    ///
    /// The file sink operates with file descriptor resources which cannot be safely copied.
    file_t(const file_t& other) = delete;

    /// Constructs a file sink using the given other file sink by moving its content.
    file_t(file_t&& other) noexcept;

    /// Destroys the current file sink instance, freeing all its resources.
    ~file_t();

    /// Copy assignment is explicitly prohibited.
    ///
    /// The file sink operates with file descriptor resources which cannot be safely copied.
    auto operator=(const file_t& other) -> file_t& = delete;

    /// Assigns the given file sink to the current one by moving its content.
    auto operator=(file_t&& other) noexcept -> file_t&;

    /// Returns a const lvalue referente to destination path pattern.
    ///
    /// The path can contain attribute placeholders, meaning that the real destination name will be
    /// deduced at runtime using provided log record. No real file will be opened at construction
    /// time.
    auto path() const -> const std::string&;

    /// Outputs the formatted message with its associated record to the file.
    ///
    /// Depending on the filename pattern it is possible to write into multiple destinations.
    auto emit(const record_t& record, const string_view& formatted) -> void;

private:
    file_t(std::unique_ptr<file::inner_t> inner) noexcept;
    file_t(std::unique_ptr<properties_t> properties);
};

class file_t::builder_t {
    std::unique_ptr<file_t::properties_t> properties;

public:
    explicit builder_t(const std::string& filename);
    builder_t(const builder_t& other) = delete;
    builder_t(builder_t&& other) noexcept;

    ~builder_t();

    auto operator=(const builder_t& other) -> builder_t& = delete;
    auto operator=(builder_t&& other) noexcept -> builder_t&;

    /// Specifies a flush interval in terms of write operations.
    ///
    /// Logging backend will flush its internal buffers after at least every count writes, but the
    /// underlying implementation can decide to do it more often. Note that 0 value means automatic
    /// policy.
    ///
    /// \param count flush interval.
    auto interval(std::size_t count) -> builder_t&;

    // TODO:
    // auto interval(bytesize_t size) -> builder_t&

    auto build() -> file_t;
};

}  // namespace sink

template<>
struct factory<sink::file_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::file_t;
};

}  // namespace v1
}  // namespace blackhole
