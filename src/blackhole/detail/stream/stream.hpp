#pragma once

#include <functional>
#include <ostream>
#include <string>

#include <boost/assert.hpp>
#include <boost/config.hpp>

#include "blackhole/detail/support/char.hpp"

#include "streambuf.hpp"

namespace blackhole {

namespace aux {

template<typename Char, class Traits = std::char_traits<Char>, class Allocator = std::allocator<Char> >
class attachable_basic_ostringstream : public std::basic_ostream<Char, Traits> {
public:
    typedef Char char_type;
    typedef Traits traits_type;
    typedef Allocator allocator_type;
    typedef basic_ostringstreambuf<Char, Traits, Allocator> streambuf_type;
    typedef typename streambuf_type::string_type string_type;

    typedef std::basic_ostream<Char, Traits> ostream_type;
    typedef typename ostream_type::int_type int_type;
    typedef typename ostream_type::pos_type pos_type;
    typedef typename ostream_type::off_type off_type;

private:
    typedef std::ios_base&(*ios_base_manip)(std::ios_base&);
    typedef std::basic_ios<char_type, traits_type>&(*basic_ios_manip)(std::basic_ios<char_type, traits_type>&);
    typedef ostream_type&(*stream_manip)(ostream_type&);

    streambuf_type streambuf;

public:
    //!@note: According to the 27.7.3.2/4 `std::basic_ostream` dtor does not perform any
    //!       operations on rdbuf(), so it can be in broken state. However there is no mention
    //!       about `std::basic_ostream` ctor behaviour on passing non-initialized `streambuf`
    //!       member pointer.
    //!       I am pretty sure it's safe, because `std::ostringstream` does the same way.
    attachable_basic_ostringstream() :
        ostream_type(&streambuf)
    {
        initialize();
    }

    explicit attachable_basic_ostringstream(string_type& string) :
        ostream_type(&streambuf),
        streambuf(std::ref(string))
    {
        initialize();
    }

    attachable_basic_ostringstream(const attachable_basic_ostringstream & other) = delete;
    attachable_basic_ostringstream& operator=(const attachable_basic_ostringstream& other) = delete;

    ~attachable_basic_ostringstream() {
        if (streambuf.storage()) {
            this->flush();
        }
    }

    void attach(string_type& string) {
        streambuf.attach(string);
        ostream_type::clear(ostream_type::goodbit);
    }

    void detach() {
        streambuf.detach();
        ostream_type::clear(ostream_type::badbit);
    }

    const string_type& str() {
        this->flush();

        string_type* storage = streambuf.storage();
        BOOST_ASSERT(storage != nullptr);

        return *storage;
    }

    attachable_basic_ostringstream& flush() {
        ostream_type::flush();
        return *this;
    }

    attachable_basic_ostringstream& seekp(pos_type pos) {
        ostream_type::seekp(pos);
        return *this;
    }

    attachable_basic_ostringstream& seekp(off_type off, std::ios_base::seekdir dir) {
        ostream_type::seekp(off, dir);
        return *this;
    }

    streambuf_type* rdbuf() {
        return &streambuf;
    }

    attachable_basic_ostringstream& put(char_type c) {
        ostream_type::put(c);
        return *this;
    }

    template<typename T>
    typename std::enable_if<supported_char<T>::value, T>::type put(T c) {
        write(boost::addressof(c), 1);
        return *this;
    }

    attachable_basic_ostringstream& write(const char_type* p, std::streamsize size) {
        ostream_type::write(p, size);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(ios_base_manip manip) {
        ostream_type::operator<<(manip);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(basic_ios_manip manip) {
        ostream_type::operator<<(manip);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(stream_manip manip) {
        ostream_type::operator<<(manip);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(char c) {
        put(c);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(const char* p) {
        write(p, static_cast<std::streamsize>(std::char_traits<char>::length(p)));
        return *this;
    }

    attachable_basic_ostringstream& operator<<(bool value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(signed char value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(unsigned char value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(short value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(unsigned short value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(int value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(unsigned int value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(long value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(unsigned long value) {
        ostream_type::operator<<(value);
        return *this;
    }

#if !defined(BOOST_NO_LONG_LONG)
    attachable_basic_ostringstream& operator<<(long long value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(unsigned long long value) {
        ostream_type::operator<<(value);
        return *this;
    }
#endif

    attachable_basic_ostringstream& operator<<(float value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(double value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(long double value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(const void* value) {
        ostream_type::operator<<(value);
        return *this;
    }

    attachable_basic_ostringstream& operator<<(std::basic_streambuf<char_type, traits_type>* buf) {
        ostream_type::operator<<(buf);
        return *this;
    }

    template<class T, class OtherTraits, class OtherAllocator >
    typename std::enable_if<supported_char<T>::value, attachable_basic_ostringstream&>::type
    operator<<(const std::basic_string<T, OtherTraits, OtherAllocator>& str) {
        write(str.c_str(), static_cast<std::streamsize>(str.size()));
        return *this;
    }

private:
    void initialize() {
        ostream_type::clear(streambuf.storage() ? ostream_type::goodbit : ostream_type::badbit);
        ostream_type::flags(ostream_type::dec | ostream_type::skipws | ostream_type::boolalpha);
        ostream_type::width(0);
        ostream_type::precision(6);
        ostream_type::fill(static_cast<char_type>(' '));
    }
};

typedef attachable_basic_ostringstream<char> attachable_ostringstream;

} // namespace aux

} // namespace blackhole
