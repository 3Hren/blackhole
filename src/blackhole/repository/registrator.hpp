#pragma once

namespace blackhole {

template<typename Level> class frontend_factory_t;

namespace aux {

namespace mpl {

template<class Sink, class Formatter> struct id {};

} // namespace mpl

namespace registrator {

template<typename Level>
struct frontend {
    frontend_factory_t<Level>& factory;

    template<typename Sink, typename Formatter>
    void operator ()(aux::mpl::id<Sink, Formatter>) const {
        factory.template add<Sink, Formatter>();
    }
};

} // namespace registrator

} // namespace aux

} // namespace blackhole
