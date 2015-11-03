#pragma once

namespace blackhole {
namespace detail {

class null_t {};

template<typename T>
union optional_storage {
    null_t null;
    T      value;

    constexpr
    optional_storage() :
        null{}
    {}

    constexpr
    optional_storage(const T& value) :
        value{value}
    {}

    ~optional_storage() = default;
};

}  // namespace detail
}  // namespace blackhole

namespace blackhole {

/// We don't want to wait for C++17, so imitate it partially.
template<typename T>
class optional {
    detail::optional_storage<T> storage;
    bool initialized;

public:
    constexpr
    optional() :
        storage{},
        initialized{false}
    {}

    constexpr
    optional(const T& value) :
        storage{value},
        initialized{true}
    {}

    constexpr
    operator bool() const noexcept {
        return initialized;
    }

    /// \pre assert(!!(*this));
    constexpr
    const T&
    operator*() const noexcept {
        return storage.value;
    }

    /// \pre assert(!!(*this));
    constexpr
    const T* const
    operator->() const noexcept {
        return &storage.value;
    }
};

}  // namespace blackhole
