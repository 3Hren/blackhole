#pragma once

#include "blackhole/detail/util/meta.hpp"

namespace blackhole {

class external_factory_t;
class frontend_factory_t;

template<class Sink, class Formatter, class = void>
struct external_inserter;

namespace aux {

namespace registrator {

struct group {
    external_factory_t& factory;

    template<class Sink, class Formatters>
    void operator()(util::metaholder<Sink, Formatters>) const {
        external_inserter<Sink, Formatters>::insert(factory);
    }
};

struct frontend {
    frontend_factory_t& factory;

    template<class Sink, class Formatter>
    void operator()(util::metaholder<Sink, Formatter>) const {
        factory.add<Sink, Formatter>();
    }
};

} // namespace registrator

} // namespace aux

} // namespace blackhole
