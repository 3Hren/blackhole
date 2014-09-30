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
    mutable std::mutex mutex;

    std::unordered_map<
        std::type_index,
        std::unordered_map<
            std::type_index,
            std::function<std::unique_ptr<base_frontend_t>(const frontend_config_t&)>
        >
    > map;

    typedef std::tuple<std::function<std::type_index(const std::string&, const dynamic_t&)>, std::type_index> tup;
    std::vector<tup> findex;
    std::vector<tup> sindex;

    template<class Formatter, class Sink>
    static
    std::unique_ptr<base_frontend_t>
    create_frontend(const frontend_config_t& config) {
        auto formatter = aux::util::make_unique<Formatter>(
            aux::config_mapper<Formatter>::map(config.formatter.config())
        );

        auto sink = aux::util::make_unique<Sink>(
            aux::config_mapper<Sink>::map(config.sink.config())
        );

        return aux::util::make_unique<
            frontend_t<Formatter, Sink>
        >(std::move(formatter), std::move(sink));
    }

public:
    template<class Sink, class Formatter>
    typename std::enable_if<
        boost::mpl::is_sequence<Sink>::type::value && boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        // do double iterations.
        anarchy action { this };
        boost::mpl::for_each<
            Sink,
            aux::util::metaholder<boost::mpl::_, Formatter>
        >(action);
    }

    template<class Sink, class Formatter>
    typename std::enable_if<
        boost::mpl::is_sequence<Sink>::type::value && !boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        // do iterations over Sink
        anarchy action { this };
        boost::mpl::for_each<
            Sink,
            aux::util::metaholder<boost::mpl::_, Formatter>
        >(action);
    }

    struct anarchy {
        external_factory_t* factory;

        template<class Sink, class Formatter>
        void operator()(aux::util::metaholder<Sink, Formatter>) const {
            factory->add<Sink, Formatter>();
        }
    };


    template<class Sink, class Formatter>
    typename std::enable_if<
        !boost::mpl::is_sequence<Sink>::type::value && boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        // do iterations over Formatter
        anarchy action { this };
        boost::mpl::for_each<
            Formatter,
            aux::util::metaholder<Sink, boost::mpl::_>
        >(action);
    }

    //!@todo: Swap type parameters.
    template<class Sink, class Formatter>
    typename std::enable_if<
        !boost::mpl::is_sequence<Sink>::type::value && !boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        auto fid = std::type_index(typeid(Formatter));
        findex.push_back(
            std::make_tuple(std::bind(&match_traits<Formatter>::ti, std::placeholders::_1, std::placeholders::_2), fid)
        );

        auto sid = std::type_index(typeid(Sink));
        sindex.push_back(
            std::make_tuple(std::bind(&match_traits<Sink>::ti, std::placeholders::_1, std::placeholders::_2), sid)
        );

        map[fid][sid] = std::bind(&create_frontend<Formatter, Sink>, std::placeholders::_1);
    }

    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) const {
        return create(frontend_config_t { formatter_config, sink_config });
    }

    std::unique_ptr<base_frontend_t>
    create(const frontend_config_t& config) const {
        auto fid = std::find_if(findex.begin(), findex.end(), [&config](const tup& t) {
            return std::get<0>(t)(config.formatter.type(), config.formatter.config()) == std::get<1>(t);
        });

        auto sid = std::find_if(sindex.begin(), sindex.end(), [&config](const tup& t) {
            return std::get<0>(t)(config.sink.type(), config.sink.config()) == std::get<1>(t);
        });

        if (fid == findex.end()) {
            throw blackhole::error_t("formatter not registered");
        }

        if (sid == sindex.end()) {
            throw blackhole::error_t("sink not registered");
        }

        auto it = map.find(std::get<1>(*fid));
        if (it == map.end()) {
            throw blackhole::error_t("formatter not registered");
        }

        auto map2 = it->second;
        auto it2 = map2.find(std::get<1>(*sid));
        if (it2 == map2.end()) {
            throw blackhole::error_t("sink not registered");
        }

        return it2->second(config);
    }

    template<class Sink, class Formatter>
    bool has() const {
        std::lock_guard<std::mutex> lock(mutex);

        auto fid = std::find_if(findex.begin(), findex.end(), [](const tup& t) {
            return std::get<1>(t) == std::type_index(typeid(Formatter));
        });

        auto sid = std::find_if(sindex.begin(), sindex.end(), [](const tup& t) {
            return std::get<1>(t) == std::type_index(typeid(Sink));
        });

        return fid != findex.end() && sid != sindex.end();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        map.clear();
    }
};

} // namespace blackhole
