#pragma once

namespace blackhole {

namespace aux {

namespace mpl {

template<class T> struct id {};

} // namespace mpl

template<typename Level, typename Sink>
struct formatter_registrator {
    template<typename Formatter>
    void operator ()(aux::mpl::id<Formatter>) const {
        formatter_factory_t<Level>::instance().template add<Sink, Formatter>();
    }
};

} // namespace aux

} // namespace blackhole
