#include "blackhole/config/option.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"

namespace blackhole {
namespace config {

option<node_t>::option() noexcept = default;
option<node_t>::option(std::unique_ptr<node_t> node) noexcept :
    node(std::move(node))
{}

option<node_t>::operator bool() const noexcept {
    return node != nullptr;
}

auto option<node_t>::unwrap() const -> boost::optional<const node_t&> {
    return to([&]() -> boost::optional<const node_t&> {
        return *node;
    });
}

auto option<node_t>::to_bool() const -> boost::optional<bool> {
    return to([&]() -> boost::optional<bool> {
        return node->to_bool();
    });
}

auto option<node_t>::to_sint64() const -> boost::optional<std::int64_t> {
    return to([&]() -> boost::optional<std::int64_t> {
        return node->to_sint64();
    });
}

auto option<node_t>::to_uint64() const -> boost::optional<std::uint64_t> {
    return to([&]() -> boost::optional<std::uint64_t> {
        return node->to_uint64();
    });
}

auto option<node_t>::to_double() const -> boost::optional<double> {
    return to([&]() -> boost::optional<double> {
        return node->to_double();
    });
}

auto option<node_t>::to_string() const -> boost::optional<std::string> {
    return to([&]() -> boost::optional<std::string> {
        return node->to_string();
    });
}

auto option<node_t>::each(const each_function& fn) -> void {
    if (node) {
        node->each(fn);
    }
}

auto option<node_t>::each_map(const member_function& fn) -> void {
    if (node) {
        node->each_map(fn);
    }
}

auto option<node_t>::operator[](const std::size_t& idx) const -> option<node_t> {
    return to([&]() -> option<node_t> {
        return (*node)[idx];
    });
}

auto option<node_t>::operator[](const std::string& key) const -> option<node_t> {
    return to([&]() -> option<node_t> {
        return (*node)[key];
    });
}

template<typename F>
auto option<node_t>::to(F&& fn) const -> decltype(fn()) {
    if (node) {
        return fn();
    } else {
        return {};
    }
}

}  // namespace config
}  // namespace blackhole
