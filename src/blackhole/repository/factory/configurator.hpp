#pragma once

#include <boost/mpl/for_each.hpp>

#include "blackhole/repository/factory/id.hpp"

namespace blackhole {

// Forward declarations.
template<typename Level> class group_factory_t;
template<typename Level> class frontend_factory_t;
namespace aux { namespace registrator { template<typename Level> struct group; } }

template<typename Sink, typename Formatter, class = void>
struct configurator {
    template<typename Level>
    static void execute(group_factory_t<Level>& factory) {
        factory.template add<Sink, Formatter>();
    }
};

template<typename Sinks, typename Formatters>
struct configurator<
    Sinks,
    Formatters,
    typename std::enable_if<
        boost::mpl::is_sequence<Sinks>::type::value &&
        boost::mpl::is_sequence<Formatters>::type::value
    >::type
> {
    template<typename Level>
    static void execute(group_factory_t<Level>& factory) {
        aux::registrator::group<Level> wrap {factory};
        boost::mpl::for_each<Sinks, aux::mpl::id<boost::mpl::_, Formatters>>(wrap);
    }
};

namespace aux {

namespace registrator {

template<typename Level>
struct group {
    group_factory_t<Level>& factory;

    template<typename Sink, typename Formatters>
    void operator ()(aux::mpl::id<Sink, Formatters>) const {
        configurator<Sink, Formatters>::execute(factory);
    }
};

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
