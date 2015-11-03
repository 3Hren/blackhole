#pragma once

#include <string>
#include <vector>

#define FMT_HEADER_ONLY
#include <cppformat/format.h>

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/sandbox.hpp"
#include "blackhole/record.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {

namespace cppformat = fmt;

class scoped_t;

class logger_t {
public:
    typedef std::function<auto(cppformat::MemoryWriter&) -> void> format_t;

public:
    virtual ~logger_t() {}

    /// Log a message with the given severity level.
    auto log(int severity, string_view format) const -> void;

    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    ///       this method.
    template<typename T, typename... Args>
    auto log(int severity, string_view format, const T& arg, const Args&... args) const -> void;

    /// Log a message with the given severity level and attributes.
    ///
    /// \overload
    auto log(int severity, const attributes_t& attributes, string_view format) const -> void;

    /// Log a message with the given severity level and attributes and further formatting using the
    /// given pattern and arguments.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    ///       this method.
    template<typename T, typename... Args>
    auto log(int severity, const attributes_t& attributes, string_view format, const T& arg, const Args&... args) const -> void;

#ifdef __cpp_constexpr
    /// Log a message with the given severity level and further formatting using the given pattern
    /// and arguments.
    ///
    /// This is more fast alternative comparing with default formatting, because the given pattern
    /// string is parsed into literals during compile time.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    ///       this method.
    template<std::size_t N, typename T, typename... Args>
    auto log(int severity, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void;

    /// Log a message with the given severity level and attributes and further formatting using the
    /// given pattern and arguments.
    ///
    /// This is more fast alternative comparing with default formatting, because the given pattern
    /// string is parsed into literals during compile time.
    ///
    /// \overload
    /// \tparam T and Args... must meet the requirements of `StreamFormatted`.
    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    ///       this method.
    template<std::size_t N, typename T, typename... Args>
    auto log(int severity, const attributes_t& attributes, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void;
#endif

    virtual auto execute(int severity, string_view format, range_t& range) const -> void = 0;
    virtual auto execute(int severity, string_view format, range_t& range, const format_t& fn) const -> void = 0;
};

inline
auto
logger_t::log(int severity, string_view format) const -> void {
    range_t range;
    this->execute(severity, format, range);
}

template<typename T, typename... Args>
inline
auto
logger_t::log(int severity, string_view format, const T& arg, const Args&... args) const -> void {
    const auto fn = [&](cppformat::MemoryWriter& wr) {
        wr.write(format.data(), arg, args...);
    };

    range_t range;
    this->execute(severity, format, range, std::cref(fn));
}

inline
auto
logger_t::log(int severity, const attributes_t& attributes, string_view format) const -> void {
    range_t range;
    range.push_back(attributes);

    this->execute(severity, format, range);
}

template<typename T, typename... Args>
inline
auto
logger_t::log(int severity, const attributes_t& attributes, string_view format, const T& arg, const Args&... args) const -> void {
    const auto fn = [&](cppformat::MemoryWriter& wr) {
        wr.write(format.data(), arg, args...);
    };

    range_t range;
    range.push_back(attributes);

    this->execute(severity, format, range, std::cref(fn));
}

#ifdef __cpp_constexpr
template<std::size_t N, typename T, typename... Args>
inline
auto
logger_t::log(int severity, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void {
    const auto fn = [&](cppformat::MemoryWriter& wr) {
        formatter.format(wr, arg, args...);
    };

    range_t range;
    this->execute(severity, "", range, std::cref(fn));
}
#endif

class root_logger_t : public logger_t {
    typedef std::function<auto(const record_t&) -> bool> filter_t;

private:
    struct inner_t;
    std::shared_ptr<inner_t> inner;

public:
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    root_logger_t(std::vector<std::unique_ptr<handler_t>> handlers);
    root_logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers);

    root_logger_t(const root_logger_t& other) = delete;
    root_logger_t(root_logger_t&& other);

    ~root_logger_t();

    auto operator=(const root_logger_t& other) -> root_logger_t& = delete;
    auto operator=(root_logger_t&& other) -> root_logger_t&;

    auto filter(filter_t fn) -> void;

    auto execute(int severity, string_view format, range_t& range) const -> void;
    auto execute(int severity, string_view format, range_t& range, const format_t& callback) const -> void;

    auto scoped(attributes_t attributes) const -> scoped_t;
};

}  // namespace blackhole
