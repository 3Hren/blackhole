#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/is_sequence.hpp>

#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
#include "blackhole/repository/factory/factory.hpp"
#include "blackhole/repository/factory/frontend.hpp"

namespace blackhole {

// Forward declarations.
class group_factory_t;
class frontend_factory_t;
namespace aux { namespace registrator { struct group; } }

template<typename Sink, typename Formatter, class = void>
struct configurator;

namespace aux {

namespace registrator {

struct group {
    group_factory_t& factory;

    template<typename Sink, typename Formatters>
    void operator ()(meta::holder<Sink, Formatters>) const {
        configurator<Sink, Formatters>::execute(factory);
    }
};

struct frontend {
    frontend_factory_t& factory;

    template<typename Sink, typename Formatter>
    void operator ()(meta::holder<Sink, Formatter>) const {
        factory.add<Sink, Formatter>();
    }
};

} // namespace registrator

} // namespace aux

template<typename Sink, typename Formatter, class = void>
struct frontend_repository {
    static void push(frontend_factory_t& factory) {
        factory.template add<Sink, Formatter>();
    }
};

template<typename Sink, typename Formatters>
struct frontend_repository<Sink, Formatters, typename std::enable_if<boost::mpl::is_sequence<Formatters>::type::value>::type> {
    static void push(frontend_factory_t& factory) {
        aux::registrator::frontend action { factory };
        boost::mpl::for_each<
            Formatters,
            meta::holder<Sink, boost::mpl::_>
        >(action);
    }
};

class group_factory_t {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef return_type(*factory_type)(const frontend_factory_t&, const formatter_config_t&, const sink_config_t&);
    typedef std::string(*extractor_type)(const boost::any&);

    mutable std::mutex mutex;
    std::unordered_map<std::string, factory_type> sinks;
    std::unordered_map<std::string, extractor_type> config_id_extractors;

    frontend_factory_t factory;
public:
    template<typename Sink, typename Formatter>
    void add() {
        std::lock_guard<std::mutex> lock(mutex);
        config_id_extractors[Sink::name()] = &unique_id_traits<Sink>::generate;
        sinks[config_traits<Sink>::name()] = static_cast<factory_type>(&factory_t::template create<Sink>);
        frontend_repository<Sink, Formatter>::push(factory);
    }

    template<typename Sink, typename Formatter>
    bool has() const {
        std::lock_guard<std::mutex> lock(mutex);
        return sinks.find(Sink::name()) != sinks.end() && factory.template has<Sink, Formatter>();
    }

    return_type create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = config_id_extractors.find(sink_config.type);
        if (it == config_id_extractors.end()) {
            throw error_t("sink '%s' is not registered", sink_config.type);
        }

        const std::string& config_name = it->second(sink_config.config);

        auto cit = sinks.find(config_name);
        if (cit == sinks.end()) {
            throw error_t("sink '%s' is not properly configured: found '%s' subtype, but other is required", sink_config.type, config_name);
        }

        const factory_type& fn = cit->second;
        return fn(factory, formatter_config, sink_config);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        sinks.clear();
        factory.clear();
    }
};

template<typename Sink, typename Formatter>
struct configurator<Sink, Formatter,
        typename std::enable_if<
            !(boost::mpl::is_sequence<Sink>::type::value &&
            boost::mpl::is_sequence<Formatter>::type::value)
        >::type> {
    static void execute(group_factory_t& factory) {
        factory.add<Sink, Formatter>();
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
    static void execute(group_factory_t& factory) {
        aux::registrator::group action { factory };
        boost::mpl::for_each<Sinks, meta::holder<boost::mpl::_, Formatters>>(action);
    }
};

} // namespace blackhole
