#pragma once

#include "blackhole/sink.hpp"

#include <memory>

namespace blackhole {

template<typename>
struct factory;

}  // namespace blackhole

namespace blackhole {
namespace config {

class node_t;

}  // namespace config
}  // namespace blackhole

namespace blackhole {
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
    explicit file_t(const std::string& filename);

    /// Copy constructing is explicitly prohibited.
    file_t(const file_t& other) = delete;

    /// Constructs a file sink using the given other file sink by moving its content.
    file_t(file_t&& other) noexcept;

    /// Destroys the current file sink instance, freeing all its resources.
    ~file_t();

    /// Copy assignment is explicitly prohibited.
    auto operator=(const file_t& other) -> file_t& = delete;

    /// Assigns the given file sink to the current one by moving its content.
    auto operator=(file_t&& other) noexcept -> file_t&;

    /// Filters the given log record determining if it is allowed to be consumed by this sink.
    ///
    /// The file sink implementation always returns `true`, meaning that all logging events should
    /// be accepted.
    auto filter(const record_t& record) -> bool;

    /// Consumes the log record with the given formatted string with its further writing to the
    /// files associated.
    auto execute(const record_t& record, const string_view& formatted) -> void;

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

    auto interval(std::size_t count) -> builder_t&;

    auto build() -> file_t;
};

}  // namespace sink

template<>
struct factory<sink::file_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> sink::file_t;
};

}  // namespace blackhole
