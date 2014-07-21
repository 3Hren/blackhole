#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/is_sequence.hpp>

#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
//!@note: Must be included first to resolve circular depencencies.
#include "blackhole/repository/factory/frontend/helper.hpp"
#include "blackhole/repository/factory/frontend.hpp"
#include "blackhole/repository/factory/frontend/inserter.hpp"
#include "blackhole/repository/factory/registrator.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

class external_factory_t {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef return_type(*factory_type)(const frontend_factory_t&, const formatter_config_t&, const sink_config_t&);
    typedef std::string(*extractor_type)(const dynamic_t&);

    mutable std::mutex mutex;
    std::unordered_map<std::string, factory_type> sinks;
    std::unordered_map<std::string, extractor_type> config_id_extractors;

    frontend_factory_t factory;
public:
    template<class Sink, class Formatter>
    void add() {
        std::lock_guard<std::mutex> lock(mutex);
        config_id_extractors[Sink::name()] = &unique_id_traits<Sink>::generate;
        sinks[config_traits<Sink>::name()] = static_cast<factory_type>(&factory::frontend::template create<Sink>);
        frontend_inserter<Sink, Formatter>::insert(factory);
    }

    template<class Sink, class Formatter>
    bool has() const {
        std::lock_guard<std::mutex> lock(mutex);
        return sinks.find(Sink::name()) != sinks.end() && factory.template has<Sink, Formatter>();
    }

    return_type create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = config_id_extractors.find(sink_config.type());
        if (it == config_id_extractors.end()) {
            throw error_t("sink '%s' is not registered", sink_config.type());
        }

        const std::string& config_name = it->second(sink_config.config());

        auto cit = sinks.find(config_name);
        if (cit == sinks.end()) {
            throw error_t("sink '%s' is not properly configured: found '%s' subtype, but other is required", sink_config.type(), config_name);
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

template<class Sinks, class Formatters>
struct all_sequence {
    static const bool value = boost::mpl::is_sequence<Sinks>::type::value &&
        boost::mpl::is_sequence<Formatters>::type::value;
};

template<class Sink, class Formatter>
struct external_inserter<
    Sink,
    Formatter,
    typename std::enable_if<!all_sequence<Sink, Formatter>::value>::type
> {
    static void insert(external_factory_t& factory) {
        factory.add<Sink, Formatter>();
    }
};

template<class Sinks, class Formatters>
struct external_inserter<
    Sinks,
    Formatters,
    typename std::enable_if<all_sequence<Sinks, Formatters>::value>::type
> {
    static void insert(external_factory_t& factory) {
        aux::registrator::group action { factory };
        boost::mpl::for_each<Sinks, meta::holder<boost::mpl::_, Formatters>>(action);
    }
};

} // namespace blackhole
