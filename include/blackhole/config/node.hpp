#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace blackhole {
inline namespace v1 {
namespace config {

template<typename T>
class option;

/// Represents the configuration tree node.
///
/// Blackhole operates with instances of this class while configuring the logging system from some
/// generic source, from file for example. It assumes that the whole configuration can be described
/// using tree data structure, like JSON, XML or YAML.
/// To be able to initialize from your own data format you must create subclass and implement its
/// converting methods as like as tree traversing using subscription operators.
class node_t {
public:
    typedef std::function<auto(const node_t& node) -> void> each_function;
    typedef std::function<auto(const std::string& key, const node_t& node) -> void> member_function;

public:
    virtual ~node_t() = 0;

    virtual auto is_bool() const noexcept -> bool = 0;
    virtual auto is_sint64() const noexcept -> bool = 0;
    virtual auto is_uint64() const noexcept -> bool = 0;
    virtual auto is_double() const noexcept -> bool = 0;
    virtual auto is_string() const noexcept -> bool { return false; }
    virtual auto is_vector() const noexcept -> bool { return false; }
    virtual auto is_object() const noexcept -> bool { return false; }

    /// Tries to convert the underlying object to bool.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    virtual auto to_bool() const -> bool = 0;

    /// Tries to convert the underlying object to signed integer.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    virtual auto to_sint64() const -> std::int64_t = 0;

    /// Tries to convert the underlying object to unsigned integer.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    virtual auto to_uint64() const -> std::uint64_t = 0;

    /// Tries to convert the underlying object to double.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    virtual auto to_double() const -> double = 0;

    /// Tries to convert the underlying object to string.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    virtual auto to_string() const -> std::string = 0;

    /// Assuming that the underlying object is an array, performs inner iteration over it by
    /// applying the given function to each element.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    /// Should do nothing if the underlying array is empty.
    virtual auto each(const each_function& fn) const -> void = 0;

    /// Assuming that the underlying object is a map, performs inner iteration over it by applying
    /// the given function to each key-value element.
    ///
    /// Implementations are free to throw exceptions on either type mismatch or whatever else.
    /// Should do nothing if the underlying map is empty.
    virtual auto each_map(const member_function& fn) const -> void = 0;

    /// Assuming that the underlying object is an array performs index operation returning the
    /// option object with some node at the given index on success, none otherwise.
    ///
    /// Depending on the concrete implementation it may or not throw exceptions on out of range
    /// access.
    virtual auto operator[](const std::size_t& idx) const -> option<node_t> = 0;

    /// Assuming that the underlying object is a map performs tree traversing operation returning
    /// option object with some node at the given key on success, none otherwise.
    ///
    /// Depending on the concrete implementation it may or not throw exceptions on out of range
    /// access.
    virtual auto operator[](const std::string& key) const -> option<node_t> = 0;
};

}  // namespace config
}  // namespace v1
}  // namespace blackhole
