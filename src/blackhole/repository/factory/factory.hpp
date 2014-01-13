#pragma once

#include <memory>

#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"

namespace blackhole {

template<typename Level>
class sink_factory_t;

template<typename Level>
class formatter_factory_t;

class base_frontend_t;

template<typename Level>
struct factory_t {
    template<typename Formatter, typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(std::unique_ptr<Formatter> formatter, std::unique_ptr<Sink> sink) {
        return std::make_unique<frontend_t<Formatter, Sink, Level>>(std::move(formatter), std::move(sink));
    }

    template<typename Formatter, typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        auto config = factory_traits<Formatter>::map_config(formatter_config.config);
        auto formatter = std::make_unique<Formatter>(config);
        formatter->set_mapper(formatter_config.mapper);
        return create(std::move(formatter), std::move(sink));
    }

    template<typename Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) {
        return formatter_factory_t<Level>::instance().create(formatter_config, std::move(sink));
    }

    template<class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        auto config = factory_traits<Sink>::map_config(sink_config.config);
        auto sink = std::make_unique<Sink>(config);
        return create(formatter_config, std::move(sink));
    }

    static
    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) {
        return sink_factory_t<Level>::instance().create(formatter_config, sink_config);
    }
};

} // nameaspace blackhole
