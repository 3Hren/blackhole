#include "blackhole/record.hpp"

#include <thread>

#include <iostream>

namespace blackhole {

struct record_t::inner_t {
    std::reference_wrapper<const string_view> message;
    std::reference_wrapper<const string_view> formatted;

    int severity;
    time_point timestamp;

    pid_t pid;
    std::thread::id tid;

    std::reference_wrapper<const attribute_pack> attributes;
};

record_t::record_t(int severity, const string_view& message, const attribute_pack& attributes) {
    static_assert(sizeof(inner_t) <= sizeof(record_t), "padding or alignment violation");

    auto& inner = this->inner();
    inner.message = message;
    inner.formatted = message;

    inner.severity = severity;
    inner.timestamp = time_point();

    inner.pid = ::getpid();

    inner.attributes = attributes;
}

auto record_t::message() const noexcept -> const string_view& {
    return inner().message.get();
}

auto record_t::severity() const noexcept -> int {
    return inner().severity;
}

auto record_t::timestamp() const noexcept -> time_point {
    return inner().timestamp;
}

auto record_t::pid() const noexcept -> std::uint64_t {
    return static_cast<std::uint64_t>(inner().pid);
}

auto record_t::tid() const noexcept -> std::uint64_t {
    return 0;
}

auto record_t::formatted() const noexcept -> const string_view& {
    return inner().formatted;
}

auto record_t::attributes() const noexcept -> const attribute_pack& {
    return inner().attributes.get();
}

auto record_t::activate(const string_view& formatted) noexcept -> void {
    if (formatted.data() != nullptr) {
        inner().formatted = formatted;
    }

    inner().timestamp = clock_type::now();
}

auto record_t::inner() noexcept -> inner_t& {
    return reinterpret_cast<inner_t&>(storage);
}

auto record_t::inner() const noexcept -> const inner_t& {
    return reinterpret_cast<const inner_t&>(storage);
}

}  // namespace blackhole
