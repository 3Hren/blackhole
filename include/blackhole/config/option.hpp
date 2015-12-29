#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <boost/optional/optional_fwd.hpp>

namespace blackhole {
namespace config {

class node_t;

template<typename T>
class option;

template<>
class option<node_t> {
public:
    typedef std::function<auto(const node_t& node) -> void> each_function;
    typedef std::function<auto(const std::string& key, const node_t& node) -> void> member_function;

private:
    std::unique_ptr<node_t> node;

public:
    /// Constructs an option object that will contan nothing.
    option() noexcept;

    /// Constructs an option object that will contain the specified configuration node.
    explicit option(std::unique_ptr<node_t> node) noexcept;

    explicit operator bool() const noexcept;

    /// Unwraps an option, yielding the content of an underlying config node object.
    ///
    /// \pre !!*this.
    /// \warning this method will probably crash your application if the value is a none.
    auto unwrap() const -> const node_t&;

    /// Unwraps an option, yielding the content of an underlying config node object.
    ///
    /// \throws std::logic_error if the value is a none with a custom message provided by `reason`.
    auto expect(std::string reason) const -> const node_t&;

    auto to_bool() const -> boost::optional<bool>;
    auto to_sint64() const -> boost::optional<std::int64_t>;
    auto to_uint64() const -> boost::optional<std::uint64_t>;
    auto to_double() const -> boost::optional<double>;
    auto to_string() const -> boost::optional<std::string>;

    auto each(const each_function& fn) -> void;
    auto each_map(const member_function& fn) -> void;

    auto operator[](const std::size_t& idx) const -> option<node_t>;
    auto operator[](const std::string& key) const -> option<node_t>;

private:
    template<typename F>
    auto to(F&& fn) const -> decltype(fn());
};

/// Constructs an option of the specified configuration node type using given arguments.
template<typename T, typename... Args>
auto make_option(Args&&... args) -> option<node_t> {
    return option<node_t>(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}  // namespace config
}  // namespace blackhole
