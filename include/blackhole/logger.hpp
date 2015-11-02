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
#include "blackhole/record.hpp"
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

typedef std::function<auto(cppformat::MemoryWriter&) -> void> format_callback;

// TODO: Rename to `logger_t`.
class logger_interface_t {
public:

public:
    virtual ~logger_interface_t() {}

    /// Entry point.
    ///
    /// \note you must include `<blakhole/extensions/format.hpp>` manually to be able to compile
    /// this method.
    template<typename... Args>
    auto log(int severity, const attributes_t& attributes, string_view format, const Args&... args) const -> void {
        log(severity, boost::make_iterator_range(attributes.begin(), attributes.end()), format, [&](cppformat::MemoryWriter& wr) {
            wr.write(format.data(), args...);
        });
    }

#ifdef __cpp_constexpr
    template<std::size_t N, typename T, typename... Args>
    void
    log(const detail::formatter<N>& formatter, const T& arg, const Args&... args) const {
        // log(0, "", [&](cppformat::MemoryWriter& wr) {
        //     formatter.format(wr, arg, args...);
        // });
        // Slow ^.

        fmt::MemoryWriter wr;
        formatter.format(wr, arg, args...);

        log(0, {wr.data(), wr.size()});
    }
#endif

    virtual auto log(int severity, string_view message) const -> void = 0;
    virtual auto log(int severity, string_view format, const format_callback& callback) const -> void = 0;
    virtual auto log(int severity, const range_type& range, string_view format, const format_callback& callback) const -> void = 0;
//  virtual auto log(int severity, const range_t& range, string_view format, const format_t& fn) const -> void = 0;
//  Better for reading ^.
};

class logger_t : public logger_interface_t {
    typedef std::function<auto(const record_t&) -> bool> filter_type;

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

    using logger_interface_t::log;

    auto log(int severity, string_view message) const -> void;
    auto log(int severity, string_view format, const format_callback& callback) const -> void;

    auto log(int severity, const range_type& range, string_view format, const format_callback& callback) const -> void override;

    void
    info(string_view message);

    void
    info(string_view message, const attributes_t& attributes);

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

    scoped_t
    scoped(attributes_t);
};

using owned_attributes_t = boost::container::small_vector<std::pair<std::string, owned_attribute_value_t>, 16>;

// TODO: Add in place wrapper (generates attributes each time instead of saving).
class wrapper_t : public logger_interface_t {
public:
    logger_interface_t& inner;
    owned_attributes_t owned;
    attributes_t attributes;

    wrapper_t(logger_interface_t& log, owned_attributes_t owned_):
        inner(log),
        owned(std::move(owned_))
    {
        for (const auto& a : owned) {
            attributes.push_back(std::make_pair(a.first, a.second));
        }
    }

    using logger_interface_t::log;

    virtual auto log(int severity, string_view message) const -> void { std::terminate(); }
    virtual auto log(int severity, string_view format, const format_callback& callback) const -> void { std::terminate(); }

    auto log(int severity, const range_type& range, string_view format, const format_callback& callback) const -> void override {
        const auto& r = this->attributes;
        inner.log(severity, boost::range::join(r, attributes), format, callback);
    }
};

}  // namespace blackhole
