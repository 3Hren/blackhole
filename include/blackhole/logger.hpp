#pragma once

#include <string>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/any_range.hpp>
#include <boost/range/join.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

#define FMT_HEADER_ONLY
#include <cppformat/format.h>

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/handler.hpp"
#include "blackhole/sandbox.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {

namespace cppformat = fmt;

using cpp17::string_view;

class scoped_t {};

using inner1_t = boost::variant<
    std::int64_t,
    double,
    string_view
>;

// mpl::transform + into_owned.
using inner2_t = boost::variant<
    std::int64_t,
    double,
    std::string
>;

class owned_attribute_value_t {
public:
    inner2_t inner;

    owned_attribute_value_t(int v):
        inner(static_cast<std::int64_t>(v))
    {}

    owned_attribute_value_t(double v):
        inner(v)
    {}

    owned_attribute_value_t(std::string v):
        inner(std::move(v))
    {}
};

struct from_owned_t : public boost::static_visitor<inner1_t> {
    template<typename T>
    inner1_t
    operator()(const T& v) const {
        return v;
    }
};

class attribute_value_t {
public:
    inner1_t inner;

    attribute_value_t(int v):
        inner(static_cast<std::int64_t>(v))
    {}

    attribute_value_t(double v):
        inner(v)
    {}

    attribute_value_t(string_view v):
        inner(v)
    {}

    attribute_value_t(const owned_attribute_value_t& v):
        inner(boost::apply_visitor(from_owned_t(), v.inner))
    {}
};

// using attributes_t = std::vector<std::pair<string_view, attribute_value_t>>;
// TODO: Try to encapsulate.
using attributes_t = boost::container::small_vector<std::pair<string_view, attribute_value_t>, 16>;

typedef boost::any_range<
    std::pair<string_view, attribute_value_t>,
    boost::forward_traversal_tag
> range_type;

class record_t {
public:
    typedef std::chrono::high_resolution_clock clock_type;
    typedef clock_type::time_point time_point;

private:
    // struct {
    //     string_view message;
    // } d;

public:
    auto message() const -> string_view;
    auto severity() const -> int;
    auto timestamp() const -> time_point;

    auto pid() const -> std::uint64_t;
    auto tid() const -> std::uint64_t;

    auto attributes() const -> const range_type&;
};

class writer_t {
public:
    string_view format;
    cppformat::MemoryWriter& wr;

    template<typename... Args>
    auto write(const Args&... args) {
        wr.write(format.data(), args...);
    }
};

class logger_t {
public:
    typedef std::function<auto(const record_t&) -> bool> filter_type;
    typedef std::function<auto(writer_t&) -> void> format_callback;

private:
    struct inner_t;
    std::shared_ptr<inner_t> inner;

public:
    /// \note you can create a logger with no handlers, it'll just drop all messages.
    logger_t(std::vector<std::unique_ptr<handler_t>> handlers);
    logger_t(filter_type filter, std::vector<std::unique_ptr<handler_t>> handlers);

    logger_t(const logger_t& other) = delete;
    logger_t(logger_t&& other);

    auto operator=(const logger_t& other) -> logger_t& = delete;
    auto operator=(logger_t&& other) -> logger_t&;

    ~logger_t();

    auto filter(filter_type fn) -> void;

    // LOG(info, "### {}", 42, ({"#1", "v2"}, {"#2", "v2"});
    // log("### {}", [](auto& stream) { stream << 42; }, {{"#1", "v1"}});

    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    /// this method.
    template<typename... Args>
    auto log(int severity, const attributes_t& range, string_view format, const Args&... args) const -> void {
        log(severity, boost::make_iterator_range(range.begin(), range.end()), format, [&](writer_t& wr) {
            wr.write(args...);
        });
    }

    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    /// this method.
    // template<typename... Args>
    // auto log(int severity, const range_type& range, string_view format, const Args&... args) const -> void {
    //     log(severity, range, format, [&](writer_t& wr) {
    //         wr.write(args...);
    //     });
    // }

    auto log(int severity, const range_type& range, string_view format, format_callback callback) const -> void;

    // auto log(string_view message, std::function<auto(writer_t& wr) -> void>, const attributes_t& attributes) const {
    //     record_t record; // Set message and attributes.
    //
    //     // if inner->filter(record) {
    //     //     // Format.
    //         cppformat::MemoryWriter wr;
    //         wr.write({message.data(), message.size()});
    //
    //         stream<cppformat::MemoryWriter> stream{wr};
    //
    //     //     // Reset message.
    //     //     // Push
    //     // }
    // }

    // log("### {}", 42, {{"#1", "v1"}});
    // // + Plain.
    // // - Hard to encapsulate cppformat (nearly impossible) -> extra compile time.
    // // - Possible excess rvalues for args or attributes.
    //
    // log("### {}", 42) << {{"#1", "v1"}};
    // // - Lazy attributes - dislike.
    //
    // log("### {}", {{"#1", "v1"}}) << 42;
    // // + Easy to encapsulate cppformat.
    // // + All attributes available for filtering.
    // // + Message args can are lazy.
    // // - Impossible to implement, because cppformat is stateless.

    // log({{"#1", "v1"}}) << format("### {}", 42);
    // // - Dislike - no improvements comparing with previous version.
    //
    // log() << format("### {}", 42) << {{"#1", "v1"}};
    // // - Dislike - need attributes.

    /// \note it's faster to provide a string literal instead of `std::string`.
    void
    info(string_view message);

    void
    info(string_view message, const attributes_t& attributes);

    void
    info_with_range(string_view message, const range_type& range) {
        for (const auto& attribute : range) {
        }
    }

    // template<typename Range>
    // void
    // info_with_range(string_view message, const Range& range) {
    //     for (const auto& attribute : range) {
    //     }
    // }

    template<typename T, typename... Args>
    void
    info(string_view message, const T& arg, const Args&... args) {
        cppformat::MemoryWriter wr;
        wr.write(message.data(), arg, args...);

        info({wr.data(), wr.size()});
    }

    template<typename T, typename... Args>
    void
    info(string_view message, const attributes_t& attributes, const T& arg, const Args&... args) {
        cppformat::MemoryWriter wr;
        wr.write(message.data(), arg, args...);

        info({wr.data(), wr.size()}, attributes);
    }

#ifdef __cpp_constexpr
    template<std::size_t N, typename T, typename... Args>
    void
    info(const detail::formatter<N>& formatter, const T& arg, const Args&... args) {
        fmt::MemoryWriter wr;
        formatter.format(wr, arg, args...);

        info({wr.data(), wr.size()});
    }
#endif

    scoped_t
    scoped(attributes_t);
};

using owned_attributes_t = boost::container::small_vector<std::pair<std::string, owned_attribute_value_t>, 16>;

// TODO: In situ wrapper (generates attributes each time instead of saving).
class wrapper_t {
public:
    logger_t& log;
    owned_attributes_t attributes;

    template<typename T, typename... Args>
    void
    info(string_view message, const attributes_t& attributes, const T& arg, const Args&... args) {
        const auto range = this->attributes |
            boost::adaptors::transformed(+[](const std::pair<std::string, owned_attribute_value_t>& v)
                -> std::pair<string_view, attribute_value_t>
            {
                return std::make_pair(v.first, v.second);
            }
        );

        // filter.

        cppformat::MemoryWriter wr;
        wr.write(message.data(), arg, args...);

        log.info_with_range({wr.data(), wr.size()}, boost::range::join(range, attributes));
    }
};

}  // namespace blackhole
