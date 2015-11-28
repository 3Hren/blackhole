#pragma once

#include <string>

#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {

template<typename T>
struct view_of {
    typedef T type;
};

template<>
struct view_of<std::string> {
    typedef string_view type;
};

}  // namespace blackhole

namespace blackhole {
namespace attribute {

class value_t;
class view_t;

/// Represents an attribute value holder.
class value_t {
public:
    typedef boost::mpl::vector<
        // std::nullptr_t
        // bool,
        std::int64_t,
        // std::uint64_t,
        double,
        std::string
    >::type types;

    typedef boost::make_variant_over<types>::type type;

private:
    /// Underlying type.
    type inner;

public:
    value_t(int val): inner(static_cast<std::int64_t>(val)) {}
    value_t(double val): inner(val) {}
    value_t(std::string val): inner(std::move(val)) {}

    /// Visitor.
    template<typename Visitor>
    auto apply(Visitor&& visitor) const -> typename Visitor::result_type {
        return boost::apply_visitor(std::forward<Visitor>(visitor), inner);
    }
};

class view_t {
public:
    typedef boost::mpl::transform<
        value_t::types,
        view_of<boost::mpl::_1>
    >::type types;

    typedef boost::make_variant_over<types>::type type;

private:
    /// Underlying type.
    type inner;

public:
    // Conversion constructors.
    view_t(int val): inner(static_cast<std::int64_t>(val)) {}
    view_t(double val): inner(val) {}
    template<std::size_t N>
    view_t(const char(&val)[N]): inner(string_view(val, N - 1)) {}
    view_t(const std::string& val): inner(string_view(val.data(), val.size())) {}
    view_t(string_view val): inner(val) {}

    /// Conversion constructor from an owned type.
    view_t(const value_t& value);

    /// Visitor.
    template<typename Visitor>
    auto apply(Visitor& visitor) const -> typename Visitor::result_type {
        return boost::apply_visitor(visitor, inner);
    }

    template<typename Visitor>
    auto apply(const Visitor& visitor) const -> typename Visitor::result_type {
        return boost::apply_visitor(visitor, inner);
    }

    auto operator==(const view_t& other) const -> bool {
        return inner == other.inner;
    }

    // operator!=
};

}  // namespace attribute
}  // namespace blackhole
