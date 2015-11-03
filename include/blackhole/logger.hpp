#pragma once

#include <string>
#include <vector>

#ifdef __has_include
#  if __has_include(<boost/container/small_vector.hpp>)
#    include <boost/container/small_vector.hpp>
#    define BLACKHOLE_HAVE_SMALL_VECTOR 1
#  else
#    define BLACKHOLE_HAVE_SMALL_VECTOR 0
#  endif
#endif

#define FMT_HEADER_ONLY
#include <cppformat/format.h>

#include "blackhole/attribute.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/sandbox.hpp"
#include "blackhole/record.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {

namespace cppformat = fmt;

using cpp17::string_view;

class scoped_t {};

// TODO: Try to encapsulate.
#ifdef BLACKHOLE_HAVE_SMALL_VECTOR
    typedef boost::container::small_vector<std::pair<string_view, attribute::value_t>, 16> attributes_t;
    typedef boost::container::small_vector<std::pair<std::string, attribute::owned_t>, 16> attributes_w_t;
#else
    typedef std::vector<std::pair<string_view, attribute::value_t>> attributes_t;
    typedef std::vector<std::pair<std::string, attribute::owned_t>> attributes_w_t;
    #warning sorry, small vector optimization is not supported
#endif

typedef boost::container::small_vector<std::reference_wrapper<const attributes_t>, 16> range_t;

typedef std::function<auto(cppformat::MemoryWriter&) -> void> format_t;

class logger_t {
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

// TODO: Add in place wrapper (generates attributes each time instead of saving).
class wrapper_t : public logger_t {
public:
    logger_t& inner;
    attributes_t attributes;
    attributes_w_t owned;

    wrapper_t(logger_t& log, attributes_w_t owned_):
        inner(log),
        owned(std::move(owned_))
    {
        for (const auto& a : owned) {
            attributes.push_back(std::make_pair(string_view(a.first.data(), a.first.size()), a.second));
        }
    }

    auto execute(int severity, string_view message, range_t& range) const -> void {}
    auto execute(int severity, string_view message, range_t& range, const format_t& fn) const -> void {
        range.push_back(attributes);
        inner.execute(severity, message, range, fn);
    }
};

}  // namespace blackhole
