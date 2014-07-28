#pragma once

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/is_sequence.hpp>

#include "blackhole/forwards.hpp"
#include "blackhole/repository/factory/registrator.hpp"
#include "blackhole/utils/meta.hpp"

namespace blackhole {

template<class Sink, class Formatter, class = void>
struct frontend_inserter {
    static void insert(frontend_factory_t& factory) {
        factory.add<Sink, Formatter>();
    }
};

template<class Sink, class Formatters>
struct frontend_inserter<
    Sink,
    Formatters,
    typename std::enable_if<boost::mpl::is_sequence<Formatters>::type::value>::type
> {
    static void insert(frontend_factory_t& factory) {
        aux::registrator::frontend action { factory };
        boost::mpl::for_each<
            Formatters,
            meta::holder<Sink, boost::mpl::_>
        >(action);
    }
};

} // namespace blackhole
