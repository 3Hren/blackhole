#pragma once

#include <iosfwd>
#include <limits>

namespace blackhole { inline namespace v2 { namespace detail {

class constexpr_string {
public:
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

private:
    const char* data_;

    /// Size without \0.
    std::size_t size_;

public:
    template<std::size_t N>
    constexpr
    constexpr_string(const char(&lit)[N]) :
        data_(lit),
        size_(N - 1)
    {}

    constexpr
    operator const char*() const noexcept {
        return data_;
    }

    constexpr
    std::size_t
    size() const noexcept {
        return size_;
    }

    constexpr
    char
    operator[](std::size_t id) const {
        if (id < size()) {
            return data_[id];
        }

        throw std::out_of_range("out of range");
    }

    std::string
    into_string() const {
        return {data_, size_};
    }

    constexpr
    constexpr_string
    substr(std::size_t pos = 0, std::size_t len = npos) const {
        if (pos > size()) {
            throw std::out_of_range("out of range");
        }

        return constexpr_string(data_ + pos, std::min(len, size() - pos));
    }

    friend
    std::ostream&
    operator<<(std::ostream& stream, const constexpr_string& value) {
        return stream.write(value.data_, static_cast<std::streamsize>(value.size_));
    }

private:
    constexpr
    constexpr_string(const char* data, std::size_t size) :
        data_(data),
        size_(size)
    {}
};

}}} // namespace blackhole::v2::detail
