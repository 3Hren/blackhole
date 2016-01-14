#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/severity.hpp"

namespace blackhole {
inline namespace v1 {

using cpp17::string_view;

class record_t {
public:
    typedef std::chrono::system_clock clock_type;
    typedef clock_type::time_point time_point;

private:
    struct inner_t;

    typedef std::aligned_storage<64>::type storage_type;
    storage_type storage;

public:
    /// Creates a log record with the given severity, possibly unformatted message and attributes.
    ///
    /// The created record contains almost all information about the logging event associated except
    /// the timestamp and formatted message.
    ///
    /// These missing attributes are set right after filtering pass with `activate` method.
    ///
    /// \warning constructing from rvalue references is explicitly forbidden, specified objects must
    ///     outlive the record created.
    record_t(severity_t severity,
        std::reference_wrapper<const string_view> message,
        std::reference_wrapper<const attribute_pack> attributes);

    auto message() const noexcept -> const string_view&;
    auto severity() const noexcept -> severity_t;
    auto timestamp() const noexcept -> time_point;

    auto pid() const noexcept -> std::uint64_t;
    auto tid() const noexcept -> std::thread::native_handle_type;

    auto formatted() const noexcept -> const string_view&;
    auto attributes() const noexcept -> const attribute_pack&;

    /// Activate the record by setting the given formatted message accompanied by obtaining and
    /// setting the current time point.
    auto activate(const string_view& formatted = string_view()) noexcept -> void;

private:
    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;
};

}  // namespace v1
}  // namespace blackhole
