#pragma once

#include <array>
#include <cstdint>
#include <streambuf>

#include <boost/assert.hpp>
#include <boost/utility/addressof.hpp>

#include "blackhole/utils/nullptr.hpp"

namespace blackhole {

namespace aux {

template<typename Char, class Traits = std::char_traits<Char>, class Allocator = std::allocator<Char> >
class basic_ostringstreambuf : public std::basic_streambuf<Char, Traits> {
    typedef std::basic_streambuf<Char, Traits> base_type;

    static const std::uint32_t INITIAL_SIZE = 16;

public:
    typedef typename base_type::char_type char_type;
    typedef typename base_type::traits_type traits_type;
    typedef typename base_type::int_type int_type;
    typedef typename base_type::pos_type pos_type;
    typedef typename base_type::off_type off_type;
    typedef std::basic_string<char_type, traits_type, Allocator> string_type;

private:
    string_type* string;
    std::array<char_type, INITIAL_SIZE> buffer;

public:
    explicit basic_ostringstreambuf() :
        string(nullptr)
    {
        base_type::setp(buffer.begin(), buffer.end());
    }

    explicit basic_ostringstreambuf(string_type& string) :
        string(boost::addressof(string))
    {
        base_type::setp(buffer.begin(), buffer.end());
    }

    basic_ostringstreambuf(const basic_ostringstreambuf& other) = delete;
    basic_ostringstreambuf& operator=(const basic_ostringstreambuf& other) = delete;

    void clear() {
        char_type* pbase = this->pbase();
        char_type* pptr = this->pptr();
        if (pbase != pptr) {
            this->pbump(static_cast<int>(pbase - pptr));
        }
    }

    void attach(string_type& string) {
        detach();
        this->string = boost::addressof(string);
    }

    void detach() {
        if (string) {
            sync();
            string = nullptr;
        }
    }

    string_type* storage() const {
        return string;
    }

protected:
    int sync() {
        BOOST_ASSERT(string != nullptr);

        char_type* pbase = this->pbase();
        char_type* pptr = this->pptr();
        if (pbase != pptr) {
            string->append(pbase, pptr);
            this->pbump(static_cast<int>(pbase - pptr));
        }

        return 0;
    }

    int_type overflow(int_type ch) {
        BOOST_ASSERT(string != nullptr);

        basic_ostringstreambuf::sync();
        if (!traits_type::eq_int_type(ch, traits_type::eof())) {
            string->push_back(traits_type::to_char_type(ch));
            return ch;
        }

        return traits_type::not_eof(ch);
    }

    std::streamsize xsputn(const char_type* s, std::streamsize n) {
        typedef typename string_type::size_type size_type;
        BOOST_ASSERT(string != nullptr);

        basic_ostringstreambuf::sync();
        const size_type max_size_left = string->max_size() - string->size();
        if (static_cast<size_type>(n) < max_size_left) {
            string->append(s, static_cast<size_type>(n));
            return n;
        }

        string->append(s, max_size_left);
        return static_cast<std::streamsize>(max_size_left);
    }
};

typedef basic_ostringstreambuf<char> ostringstreambuf;

} // namespace aux

} // namespace blackhole
