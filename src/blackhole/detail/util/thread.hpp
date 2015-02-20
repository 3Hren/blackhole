#pragma once

#include <thread>
#include <iomanip>

#include <boost/thread.hpp>

#include "blackhole/config.hpp"

#include "lazy.hpp"

BLACKHOLE_BEG_NS

namespace this_thread {

template<typename T>
inline const T& get_id() {
    static_assert(
        aux::util::lazy_false<T>::value,
        "function `get_id` for this type is not implemented."
    );
}

template<>
inline const std::string& get_id<std::string>() {
    static boost::thread_specific_ptr<std::string> instance;
    if (!instance.get()) {
        std::ostringstream stream;
#ifdef __linux
        stream << std::hex << std::internal << std::showbase << std::setw(2) << std::setfill('0');
#endif
        stream << std::this_thread::get_id();
        instance.reset(new std::string(stream.str()));
    }

    return *instance.get();
}

} // namespace this_thread

BLACKHOLE_END_NS
