#pragma once

#include <sys/time.h>

#include <ostream>

template<class Char, class Traits>
inline std::basic_ostream<Char, Traits>& operator <<(std::basic_ostream<Char, Traits>& stream, const timeval& tv) {
    stream << tv.tv_sec << "." << tv.tv_usec;
    return stream;
}

inline bool operator ==(const timeval& lhs, const timeval& rhs) {
    return lhs.tv_sec == rhs.tv_sec && lhs.tv_usec == rhs.tv_usec;
}
