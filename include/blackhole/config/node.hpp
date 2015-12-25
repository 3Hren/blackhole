#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace blackhole {
namespace config {

template<typename T>
class option;

/// Represents the configuration tree node.
class node_t {
public:
    typedef std::function<auto(const node_t& node) -> void> each_function;
    typedef std::function<auto(const std::string& key, const node_t& node) -> void> member_function;

public:
    virtual ~node_t() = 0;

    /// Tries to convert the underlying object to bool.
    virtual auto to_bool() const -> bool = 0;

    /// Tries to convert the underlying object to signed integer.
    virtual auto to_sint64() const -> std::int64_t = 0;

    /// Tries to convert the underlying object to unsigned integer.
    virtual auto to_uint64() const -> std::uint64_t = 0;

    /// Tries to convert the underlying object to double.
    virtual auto to_double() const -> double = 0;

    /// Tries to convert the underlying object to string.
    virtual auto to_string() const -> std::string = 0;

    /// Assuming that the underlying object is an array, performs inner iteration over it by
    /// applying the given function to each element.
    ///
    /// Should do nothing either if there is no underlying array or it is empty.
    virtual auto each(const each_function& fn) -> void = 0;

    /// Assuming that the underlying object is a map, performs inner iteration over it by applying
    /// the given function to each key-value element.
    ///
    /// Should do nothing either if there is no underlying map or it is empty.
    virtual auto each_map(const member_function& fn) -> void = 0;

    virtual auto operator[](const std::size_t& idx) const -> option<node_t> = 0;
    virtual auto operator[](const std::string& key) const -> option<node_t> = 0;
};

}  // namespace config
}  // namespace blackhole
