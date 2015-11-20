#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

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
    record_t(int severity, const string_view& message, const attribute_pack& attributes);

    auto message() const noexcept -> const string_view&;
    auto severity() const noexcept -> int;
    auto timestamp() const noexcept -> time_point;

    auto pid() const noexcept -> std::uint64_t;
    auto tid() const noexcept -> std::thread::id;

    auto formatted() const noexcept -> const string_view&;
    auto attributes() const noexcept -> const attribute_pack&;

    /// Activate the record by setting the given formatted message accompanied by obtaining and
    /// setting the current time point.
    auto activate(const string_view& formatted = string_view()) noexcept -> void;

private:
    // Constructing from rvalue references is explicitly forbidden.
    // TODO: Consider more elegant solution.
    record_t(int severity, const string_view&, attribute_pack&&) = delete;
    record_t(int severity, string_view&&, const attribute_pack&) = delete;
    record_t(int severity, string_view&&, attribute_pack&&) = delete;

    auto inner() noexcept -> inner_t&;
    auto inner() const noexcept -> const inner_t&;
};

}  // namespace blackhole
