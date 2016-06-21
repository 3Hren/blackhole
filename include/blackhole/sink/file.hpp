#pragma once

#include <memory>
#include <ratio>

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

/// Represents a binary unit.
// TODO: Should it be here?
template<class Rep, class Ratio = std::ratio<1>>
class binary_unit;

/// Disable all fractional units.
template<class Rep, std::uintmax_t Denom>
class binary_unit<Rep, std::ratio<1, Denom>>;

template<class Rep>
class binary_unit<Rep, std::ratio<1>> {
public:
    typedef Rep rep;

private:
    rep value;

public:
    constexpr binary_unit(rep value) noexcept :
        value(value)
    {}

    constexpr auto count() const noexcept -> rep {
        return value;
    }
};

template<class Rep, class Ratio>
class binary_unit {
public:
    typedef Rep rep;

private:
    rep value;

public:
    constexpr binary_unit(rep value) noexcept :
        value(value)
    {}

    constexpr auto count() const noexcept -> rep {
        return value;
    }

    constexpr operator binary_unit<std::uintmax_t>() noexcept {
        return binary_unit<std::uintmax_t>(count() * Ratio::num);
    }
};

typedef binary_unit<std::uintmax_t> bytes_t;
typedef binary_unit<std::uintmax_t, std::kilo> kilobytes_t;
typedef binary_unit<std::uintmax_t, std::mega> megabytes_t;
typedef binary_unit<std::uintmax_t, std::giga> gigabytes_t;
typedef binary_unit<std::uintmax_t, std::ratio<1024>> kibibytes_t;
typedef binary_unit<std::uintmax_t, std::ratio<1024 * 1024>> mibibytes_t;
typedef binary_unit<std::uintmax_t, std::ratio<1024 * 1024 * 1024>> gibibytes_t;

/// Represents a file sink builder to ease its configuration.
template<>
class builder<sink::file_t> {
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> p;

public:
    /// Constructs a file sink builder with the given file path pattern.
    ///
    /// By default this builder will produce file sinks with automatic flush policy, which can be
    /// changed using threshold methods.
    explicit builder(const std::string& path);

    /// Specifies flush threshold in terms of bytes written.
    ///
    /// Logging backend will flush its internal buffers after at least every count bytes written,
    /// but the underlying implementation can decide to do it more often.
    ///
    /// \note setting zero value resets the policy with automatic mode.
    ///
    /// \param bytes flush threshold.
    auto flush_every(bytes_t bytes) & -> builder&;
    auto flush_every(bytes_t bytes) && -> builder&&;

    /// Specifies flush threshold in terms of number of logging events processed.
    ///
    /// Logging backend will flush its internal buffers after at least every count writes, but the
    /// underlying implementation can decide to do it more often.
    ///
    /// \note setting zero value resets the policy with automatic mode.
    ///
    /// \param events flush threshold.
    auto flush_every(std::size_t events) & -> builder&;
    auto flush_every(std::size_t events) && -> builder&&;

    /// Consumes this builder, returning a newly created file sink with the options configured.
    auto build() && -> std::unique_ptr<sink_t>;
};

template<>
class factory<sink::file_t> : public factory<sink_t> {
    const registry_t& registry;

public:
    constexpr explicit factory(const registry_t& registry) noexcept :
        registry(registry)
    {}

    auto type() const noexcept -> const char*;
    auto from(const config::node_t& config) const -> std::unique_ptr<sink_t>;
};

}  // namespace v1
}  // namespace blackhole
