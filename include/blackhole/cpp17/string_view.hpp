#pragma once

#include <cstddef>
#include <string>

namespace blackhole {
namespace cpp17 {

class string_view {
    const char* data_;
    std::size_t size_;

public:
    /// Constructs an empty `string_view`.
    constexpr string_view() noexcept:
        data_(nullptr),
        size_(0)
    {}

    template<std::size_t N>
    constexpr string_view(const char(&literal)[N]) noexcept:
        data_(literal),
        size_(N - 1)
    {}

    constexpr
    string_view(const char* literal, std::size_t size) noexcept:
        data_(literal),
        size_(size)
    {}

    string_view(const std::string& message) noexcept:
        data_(message.data()),
        size_(message.size())
    {}

    constexpr
    string_view(const string_view& other) = default;

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

}  // namespace cpp17
}  // namespace blackhole
