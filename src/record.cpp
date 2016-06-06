#include "blackhole/record.hpp"

#include <thread>

#if defined(_WIN32)
#include <process.h>
#include <Windows.h>
#endif

#include "blackhole/attribute.hpp"

#include "blackhole/detail/record.hpp"

namespace blackhole {
inline namespace v1 {

record_t::record_t(severity_t severity,
    std::reference_wrapper<const string_view> message,
    std::reference_wrapper<const attribute_pack> attributes)
{
    static_assert(sizeof(inner_t) <= sizeof(record_t), "padding or alignment violation");

    auto& inner = this->inner();
    inner.message = message;
    inner.formatted = message;

    inner.severity = severity;
    inner.timestamp = time_point();

	// TODO: Decompose.
#if defined(__linux__) || defined(__APPLE__)
    inner.tid = ::pthread_self();
#elif defined(_WIN32)
	inner.tid = ::GetCurrentThread();
#else
#error "Unsupported platform"
#endif

    inner.attributes = attributes;
}

record_t::record_t(inner_t inner) noexcept {
    this->inner() = std::move(inner);
}

auto record_t::message() const noexcept -> const string_view& {
    return inner().message.get();
}

auto record_t::severity() const noexcept -> severity_t {
    return inner().severity;
}

auto record_t::timestamp() const noexcept -> time_point {
    return inner().timestamp;
}

auto record_t::pid() const noexcept -> std::uint64_t {
	// TODO: Decompose.
#if defined(__linux__) || defined(__APPLE__)
	return static_cast<std::uint64_t>(::getpid());
#elif defined(_WIN32)
	return static_cast<std::uint64_t>(::_getpid());
#else
#error "Unsupported platform"
#endif
}

auto record_t::tid() const noexcept -> std::thread::native_handle_type {
    return inner().tid;
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

    // TODO: I can use coarse clock in linux.
    inner().timestamp = clock_type::now();
}

auto record_t::inner() noexcept -> inner_t& {
    return reinterpret_cast<inner_t&>(storage);
}

auto record_t::inner() const noexcept -> const inner_t& {
    return reinterpret_cast<const inner_t&>(storage);
}

}  // namespace v1
}  // namespace blackhole
