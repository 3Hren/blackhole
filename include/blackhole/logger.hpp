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
#error sorry, small vector optimization is not supported
#endif

typedef boost::container::small_vector<std::reference_wrapper<const attributes_t>, 16> range_t;

typedef std::function<auto(cppformat::MemoryWriter&) -> void> format_t;

// TODO: Rename to `logger_t`.
class logger_interface_t {
public:
    virtual ~logger_interface_t() {}

    template<typename T, typename... Args>
    auto log(int severity, string_view format, const T& arg, const Args&... args) const -> void {
        const auto fn = [&](cppformat::MemoryWriter& wr) {
            wr.write(format.data(), arg, args...);
        };

        range_t range;
        _log(severity, format, std::cref(fn), range);
    }

    /// Entry point.
    ///
    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    /// this method.
    template<typename... Args>
    auto log(int severity, const attributes_t& attributes, string_view format, const Args&... args) const -> void {
        const auto fn = [&](cppformat::MemoryWriter& wr) {
            wr.write(format.data(), args...);
        };

        range_t range;
        range.push_back(attributes);
        _log(severity, format, std::cref(fn), range);
    }

#ifdef __cpp_constexpr
    template<std::size_t N, typename T, typename... Args>
    auto log(int severity, const detail::formatter<N>& formatter, const T& arg, const Args&... args) const -> void {
        const auto fn = [&](cppformat::MemoryWriter& wr) {
            formatter.format(wr, arg, args...);
        };

        _log(severity, "", std::cref(fn));
    }
#endif

    virtual auto log(int severity, string_view format) const -> void = 0;
    virtual auto _log(int severity, string_view format, const format_t& fn) const -> void = 0;
    virtual auto _log(int severity, string_view format, const format_t& fn, range_t& range) const -> void = 0;
};

class logger_t : public logger_interface_t {
    typedef std::function<auto(const record_t&) -> bool> filter_t;

private:
    struct inner_t;
    std::shared_ptr<inner_t> inner;

public:
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    logger_t(std::vector<std::unique_ptr<handler_t>> handlers);
    logger_t(filter_t filter, std::vector<std::unique_ptr<handler_t>> handlers);

    logger_t(const logger_t& other) = delete;
    logger_t(logger_t&& other);

    ~logger_t();

    auto operator=(const logger_t& other) -> logger_t& = delete;
    auto operator=(logger_t&& other) -> logger_t&;

    auto filter(filter_t fn) -> void;

    auto log(int severity, string_view format) const -> void;
    using logger_interface_t::log;

    scoped_t
    scoped(attributes_t);

    auto _log(int severity, string_view format, const format_t& fn) const -> void;
    auto _log(int severity, string_view format, const format_t& callback, range_t& range) const -> void;
};

// TODO: Add in place wrapper (generates attributes each time instead of saving).
class wrapper_t : public logger_interface_t {
public:
    logger_interface_t& inner;
    attributes_t attributes;
    attributes_w_t owned;

    wrapper_t(logger_interface_t& log, attributes_w_t owned_):
        inner(log),
        owned(std::move(owned_))
    {
        for (const auto& a : owned) {
            attributes.push_back(std::make_pair(string_view(a.first.data(), a.first.size()), a.second));
        }
    }

    using logger_interface_t::log;

    auto log(int severity, string_view message) const -> void {
        (void)severity;
        (void)message;
        std::terminate();
    }

    auto _log(int severity, string_view format, const format_t& fn) const -> void {
        (void)severity;
        (void)format;
        (void)fn;
        std::terminate();
    }

    auto _log(int severity, string_view format, const format_t& fn, range_t& range) const -> void {
        range.push_back(attributes);
        inner._log(severity, format, fn, range);
    }
};

}  // namespace blackhole
