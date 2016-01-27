#pragma once

#include <iosfwd>
#include <limits>
#include <stdexcept>
#include <string>

namespace blackhole {
inline namespace v1 {
namespace cpp17 {

/// The class template `basic_string_view` describes an object that can refer to a constant
/// contiguous sequence of char-like objects with the first element of the sequence at position
/// zero.
///
/// A typical implementation holds only two members: a pointer to constant Char and a size.
///
/// \tparam Char - character type.
/// \tparam Traits - traits class specifying the operations on the character type.
template<typename Char, typename Traits = std::char_traits<Char>>
class basic_string_view {
public:
    /// This is a special value equal to the maximum value representable by the type size_type.
    ///
    /// The exact meaning depends on context, but it is generally used either as end of view
    /// indicator by the functions that expect a view index or as the error indicator by the
    /// functions that return a view index.
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    typedef Char value_type;
    typedef Traits traits_type;
    typedef Char* pointer;
    typedef const Char* const_pointer;
    typedef Char& reference;
    typedef const Char& const_reference;

    typedef std::size_t size_type;

private:
    const_pointer data_;

    /// Size without \0.
    size_type size_;

public:
    /// Constructs an empty string view.
    constexpr basic_string_view() noexcept:
        data_(nullptr),
        size_(0)
    {}

    /// Constructs a view of the null-terminated character string pointed to by `literal`, not
    /// including the terminating null character.
    template<size_type N>
    constexpr basic_string_view(const Char(&literal)[N]) noexcept :
        data_(literal),
        size_(N - 1)
    {}

    /// Constructs a view of the first `size` characters of the character array starting with the
    /// element pointed by `literal`.
    ///
    /// The `literal` can contain null characters.
    /// The behavior is undefined if [literal, literal + size) is not a valid range (even though the
    /// constructor may not access any of the elements of this range).
    constexpr basic_string_view(const Char* literal, std::size_t size) noexcept :
        data_(literal),
        size_(size)
    {}

    /// Constructs a view of the first `string.size()` characters of the character array starting
    /// with the element pointed by `string.data()`.
    template<typename Allocator>
    basic_string_view(const std::basic_string<Char, Traits, Allocator>& string) noexcept :
        data_(string.data()),
        size_(string.size())
    {}

    /// Constructs a view of the same content as other.
    constexpr basic_string_view(const basic_string_view& other) = default;

    /// Replaces the view with the given other view.
    auto operator=(const basic_string_view& other) -> basic_string_view& = default;

    /// Returns a pointer to the underlying character array.
    ///
    /// The pointer is such that the range [data(); data() + size()) is valid and the values in it
    /// correspond to the values of the view.
    ///
    /// \note unlike `basic_string::data()` and string literals, `data()` may return a pointer to a
    /// buffer that is not null-terminated. Therefore it is typically a mistake to pass `data()` to
    /// a routine that takes just a const Char* and expects a null-terminated string.
    constexpr auto data() const noexcept -> const_pointer {
        return data_;
    }

    /// Returns the number of Char elements in the view.
    constexpr auto size() const noexcept -> size_type {
        return size_;
    }

    /// Returns a const reference to the character at specified location `id`.
    constexpr auto operator[](std::size_t id) const -> char {
        return id < size() ?
            data_[id] :
            throw std::out_of_range("out of range");
    }

    /// Operations.

    /// Creates a `basic_string` with a copy of the content of the current view.
    template<class Allocator = std::allocator<Char>>
    auto to_string(const Allocator& alloc = Allocator()) const ->
        std::basic_string<Char, Traits, Allocator>
    {
        return {data(), size(), alloc};
    }

    /// Returns a view of the substring [pos, pos + rcount), where rcount is the smaller of count
    /// and `size() - pos`.
    constexpr
    auto substr(std::size_t pos = 0, std::size_t count = npos) const -> basic_string_view {
        return pos <= size() ?
            basic_string_view(data() + pos, std::min(count, size() - pos)) :
            throw std::out_of_range("out of range");
    }

    /// Compares two views.
    ///
    /// All comparisons are done via the `compare()` member function, which itself is defined in
    /// terms of `Traits::compare()`.
    /// Two views are equal if both the size of `this` and `other` are equal and each character in
    /// `this` has an equivalent character in `other` at the same position.
    constexpr auto operator==(const basic_string_view& other) const noexcept -> bool {
        return size() == other.size() && traits_type::compare(data(), other.data(), size()) == 0;
    }
};

template<typename Char, typename Traits >
constexpr auto operator<(basic_string_view<Char, Traits> lhs, basic_string_view<Char, Traits> rhs) ->
    bool
{
    return std::lexicographical_compare(lhs.data(), lhs.data() + lhs.size(),
        rhs.data(), rhs.data() + rhs.size());
}

template<class Char, class Traits>
auto operator<<(std::basic_ostream<Char, Traits>& stream, const basic_string_view<Char, Traits>& value) ->
    std::basic_ostream<Char, Traits>&
{
    return stream << value.to_string();
}

/// Several typedefs for common character types are provided:
typedef basic_string_view<char> string_view;

}  // namespace cpp17

using cpp17::string_view;

}  // namespace v1
}  // namespace blackhole
