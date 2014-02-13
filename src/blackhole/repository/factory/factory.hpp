#pragma once

#include <memory>

#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

template<typename Level>
class frontend_factory_t;

class base_frontend_t;

namespace aux {
//!@todo: Move into separate file!
template<class T>
struct config_mapper {
    typedef typename T::config_type config_type;

    static config_type map(const boost::any& input) {
        const aux::extractor<T> extractor(input);
        config_type config;
        factory_traits<T>::map_config(extractor, config);
        return config;
    }
};

} // namespace aux

template<typename Level>
struct factory_t {
    template<class Formatter, class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) {
        return std::make_unique<frontend_t<Formatter, Sink, Level>>(std::move(formatter), std::move(sink));
    }

    template<class Formatter, class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        auto config = aux::config_mapper<Formatter>::map(formatter_config.config);
        auto formatter = std::make_unique<Formatter>(config);
        formatter->set_mapper(formatter_config.mapper);
        return create(std::move(formatter), std::move(sink));
    }

    template<class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const frontend_factory_t<Level>& factory, const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        return factory.create(formatter_config, std::move(sink));
    }

    template<class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const frontend_factory_t<Level>& factory, const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        auto config = aux::config_mapper<Sink>::map(sink_config.config);
        auto sink = std::make_unique<Sink>(config);
        return create(factory, formatter_config, std::move(sink));
    }
};

} // nameaspace blackhole
