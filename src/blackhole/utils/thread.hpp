#pragma once

#include <pthread.h>

#include <boost/integer.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/thread.hpp>

#include "noexcept.hpp"

namespace blackhole {

namespace generic {

template<typename T>
class id {
public:
    typedef typename T::native_type native_type;

    id(native_type id) :
        m_id(id)
    {}

    native_type native_id() const BLACKHOLE_NOEXCEPT {
        return m_id;
    }

private:
    native_type m_id;
};

} // namespace generic

struct thread_t {
    typedef uintmax_t native_type;
    typedef generic::id<thread_t> id;
};

union caster_t {
    thread_t::native_type as_uint;
    pthread_t as_pthread;
};

enum {
    tid_size = sizeof(pthread_t) > sizeof(uintmax_t) ? sizeof(uintmax_t) : sizeof(pthread_t)
};

template<typename Char, typename Traits>
std::basic_ostream<Char, Traits>& operator <<(std::basic_ostream<Char, Traits>& stream, const thread_t::id& tid) {
    if (stream.good()) {
        boost::io::ios_flags_saver flags(stream, (stream.flags() & std::ios_base::uppercase) | std::ios_base::hex | std::ios_base::internal | std::ios_base::showbase);
        boost::io::ios_width_saver width(stream, static_cast<std::streamsize>(2));
        boost::io::basic_ios_fill_saver<Char, Traits> fill(stream, static_cast<Char>('0'));
        stream << static_cast<boost::uint_t<tid_size * 8>::least>(tid.native_id());
    }

    return stream;
}

namespace this_thread {

template<typename T>
inline const T& get_id() {
    static_assert(sizeof(T) == -1, "Function `get_id` for this type is not implemented.");
}

template<>
inline const thread_t::id& get_id<thread_t::id>() {
    static boost::thread_specific_ptr<thread_t::id> instance;
    if (!instance.get()) {
        caster_t caster = {};
        caster.as_pthread = pthread_self();
        instance.reset(new thread_t::id(caster.as_uint));
    }

    return *instance.get();
}

template<>
inline const std::string& get_id<std::string>() {
    static boost::thread_specific_ptr<std::string> instance;
    if (!instance.get()) {
        std::ostringstream stream;
        stream << get_id<thread_t::id>();
        instance.reset(new std::string(stream.str()));
    }

    return *instance.get();
}

} // namespace this_thread

} // namespace blackhole
