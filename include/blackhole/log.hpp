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

#include "blackhole/sandbox.hpp"

namespace blackhole {

namespace cppformat = fmt;

class scoped_t {};

class string_view {
    const char* data_;
    std::size_t size_;

public:
    template<std::size_t N>
    constexpr
    string_view(const char(&message)[N]) noexcept:
        data_(message),
        size_(N - 1)
    {}

    constexpr
    string_view(const char* message, std::size_t size) noexcept:
        data_(message),
        size_(size)
    {}

    string_view(const std::string& message) noexcept:
        data_(message.data()),
        size_(message.size())
    {}

    constexpr
    const char*
    data() const noexcept {
        return data_;
    }

    constexpr
    std::size_t
    size() const noexcept {
        return size_;
    }
};

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

using inner1_t = boost::variant<
    std::int64_t,
    double,
    string_view
>;

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
using attributes_t = boost::container::small_vector<std::pair<string_view, attribute_value_t>, 16>;

class log_t {
    struct inner_t;
    std::unique_ptr<inner_t> inner;

public:
    log_t();
    ~log_t();

    /// \note it's faster to provide a string literal instead of `std::string`.
    void
    info(string_view message);

    void
    info(string_view message, const attributes_t& attributes);

    void
    info_with_range(string_view message, const boost::any_range<std::pair<string_view, attribute_value_t>, boost::forward_traversal_tag>& range) {
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

class wrapper_t {
public:
    log_t& log;
    owned_attributes_t attributes;

    template<typename T, typename... Args>
    void
    info(string_view message, const attributes_t& attributes, const T& arg, const Args&... args) {
        cppformat::MemoryWriter wr;
        wr.write(message.data(), arg, args...);

        const auto range = this->attributes |
            boost::adaptors::transformed(+[](const std::pair<std::string, owned_attribute_value_t>& v)
                -> std::pair<string_view, attribute_value_t>
            {
                return std::make_pair(v.first, v.second);
            }
        );

        log.info_with_range({wr.data(), wr.size()}, boost::range::join(range, attributes));
    }
};

}  // namespace blackhole
