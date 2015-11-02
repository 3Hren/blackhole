#pragma once

#include <iosfwd>
#include <limits>

namespace blackhole {
namespace cpp17 {

class string_view {
public:
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

private:
    const char* data_;
    /// Size without \0.
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
    auto data() const noexcept -> const char* {
        return data_;
    }

    constexpr
    auto size() const noexcept -> std::size_t {
        return size_;
    }

    constexpr
    auto operator[](std::size_t id) const -> char {
        if (id < size()) {
            return data_[id];
        }

        throw std::out_of_range("out of range");
    }

    /// Creates a `std::string` with a copy of the content of the current view.
    auto to_string() const -> std::string {
        return {data(), size()};
    }

    /// Returns a view of the substring [pos, pos + rcount), where rcount is the smaller of count
    /// and `size() - pos`.
    constexpr
    auto substr(std::size_t pos = 0, std::size_t count = npos) const -> string_view {
        if (pos > size()) {
            throw std::out_of_range("out of range");
        }

        return string_view(data() + pos, std::min(count, size() - pos));
    }

    friend
    std::ostream&
    operator<<(std::ostream& stream, const string_view& value) {
        return stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
};

}  // namespace cpp17

using cpp17::string_view;

}  // namespace blackhole
