#pragma once

namespace blackhole {

template<typename Level> class frontend_factory_t;

namespace aux {

namespace mpl {

template<class T> struct id {};

} // namespace mpl

template<typename Level, typename Sink>
struct formatter_registrator {
    frontend_factory_t<Level>& factory;

    template<typename Formatter>
    void operator ()(aux::mpl::id<Formatter>) const {
        factory.template add<Sink, Formatter>();
    }
};

} // namespace aux

} // namespace blackhole
