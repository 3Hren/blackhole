#pragma once

#include <ostream>

namespace blackhole {
namespace detail {
namespace datetime {

template<typename Stream, typename Char, class Traits = std::char_traits<Char>>
class basic_ostreambuf_adapter : public std::basic_streambuf<Char, Traits> {
    typedef std::basic_streambuf<Char, Traits> base_type;

public:
    typedef Stream stream_type;
    typedef typename base_type::int_type int_type;
    typedef typename base_type::char_type char_type;
    typedef typename base_type::traits_type traits_type;

private:
    stream_type& stream;

public:
    basic_ostreambuf_adapter(stream_type& stream) :
        stream(stream)
    {
        base_type::setp(nullptr, nullptr);
    }

    auto overflow(int_type ch) -> int_type {
        if (!traits_type::eq_int_type(ch, traits_type::eof())) {
            stream << traits_type::to_char_type(ch);
            return ch;
        }

        return traits_type::not_eof(ch);
    }

    auto inner() -> stream_type& {
        return stream;
    }
};

template<typename Stream>
class ostream_adapter : public std::basic_ostream<char> {
    typedef Stream stream_type;

    basic_ostreambuf_adapter<stream_type, char> streambuf;

public:
    ostream_adapter(stream_type& stream) :
        std::basic_ostream<char>(&streambuf),
        streambuf(stream)
    {}

    template<typename T>
    auto operator<<(const T& value) -> ostream_adapter& {
        streambuf.inner() << value;
        return *this;
    }
};

}  // namespace datetime
}  // namespace detail
}  // namespace blackhole
