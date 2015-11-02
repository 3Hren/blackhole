#pragma once

#include <iosfwd>
#include <limits>
#include <stdexcept>

namespace blackhole {
namespace cpp17 {

template<typename Char, typename Traits = std::char_traits<Char>>
class basic_string_view {
public:
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    typedef Char value_type;
    typedef Traits traits_type;

private:
    const char* data_;
    /// Size without \0.
    std::size_t size_;

public:
    /// Constructs an empty `string_view`.
    constexpr
    basic_string_view() noexcept:
        data_(nullptr),
        size_(0)
    {}

    template<std::size_t N>
    constexpr
    basic_string_view(const char(&literal)[N]) noexcept:
        data_(literal),
        size_(N - 1)
    {}

    constexpr
    basic_string_view(const char* literal, std::size_t size) noexcept:
        data_(literal),
        size_(size)
    {}

    basic_string_view(const std::string& message) noexcept:
        data_(message.data()),
        size_(message.size())
    {}

    constexpr
    basic_string_view(const basic_string_view& other) = default;

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

    /// Operations.

    /// Creates a `std::string` with a copy of the content of the current view.
    template<class Allocator = std::allocator<Char>>
    auto to_string(const Allocator& alloc = Allocator()) const -> std::basic_string<Char, Traits, Allocator> {
        return {data(), size(), alloc};
    }

    /// Returns a view of the substring [pos, pos + rcount), where rcount is the smaller of count
    /// and `size() - pos`.
    constexpr
    auto substr(std::size_t pos = 0, std::size_t count = npos) const -> basic_string_view {
        if (pos > size()) {
            throw std::out_of_range("out of range");
        }

        return basic_string_view(data() + pos, std::min(count, size() - pos));
    }
};

template<class Char, class Traits>
std::basic_ostream<Char, Traits>&
operator<<(std::basic_ostream<Char, Traits>& stream, const basic_string_view<Char, Traits>& value) {
    return stream << value.to_string();
}

typedef basic_string_view<char> string_view;

}  // namespace cpp17

using cpp17::string_view;

}  // namespace blackhole
