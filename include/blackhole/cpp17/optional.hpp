#pragma once

namespace blackhole {
namespace detail {
namespace std17 {

class null_t {};

template<typename T>
union optional_storage {
    null_t null;
    T value;

    constexpr optional_storage() noexcept : null{} {}
    constexpr optional_storage(const T& value) : value{value} {}

    ~optional_storage() = default;
};

}  // namespace std17
}  // namespace detail
}  // namespace blackhole

namespace blackhole {
namespace std17 {

/// Represents an empty class type used to indicate optional type with uninitialized state.
///
/// In particular, `blackhole::std17::optional` has a constructor with nullopt_t as a single
/// argument, which creates an optional that does not contain a value.
struct nullopt_t {
    constexpr nullopt_t(int) {}
};

/// Represents a constant of type `blackhole::std17::nullopt_t` that is used to indicate optional
/// type with uninitialized state.
constexpr nullopt_t nullopt{0};

/// We don't want to wait for C++17, so imitate it partially.
template<typename T>
class optional {
    detail::std17::optional_storage<T> storage;
    bool initialized;

public:
    /// Constructs the object that *does not contain a value*.
    constexpr optional() noexcept :
        storage{},
        initialized{false}
    {}

    /// Constructs an optional object that contains a value.
    ///
    /// Initialization takes palce as if direct-initializing (but not direct-list-initializing) an
    /// object of type T with the expression value. This constructor is constexpr if the constructor
    /// of T selected by direct-initialization is constexpr.
    ///
    /// \throws any exception thrown by the constructor of T.
    constexpr optional(const T& value) :
        storage{value},
        initialized{true}
    {}

    /// Checks whether `*this` contains a value.
    constexpr explicit operator bool() const noexcept {
        return initialized;
    }

    /// Returns a reference to the contained value.
    ///
    /// The behavior is undefined if `*this` *does not contain a value*.
    /// \pre assert(!!(*this));
    constexpr auto operator*() const noexcept -> const T& {
        return storage.value;
    }

    /// Returns a pointer to the contained value.
    ///
    /// The behavior is undefined if `*this` *does not contain a value*.
    /// \pre assert(!!(*this));
    constexpr auto operator->() const noexcept -> const T* {
        return &storage.value;
    }
};

}  // namespace std17
}  // namespace blackhole
