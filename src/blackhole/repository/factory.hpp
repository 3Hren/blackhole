#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/is_sequence.hpp>

#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

class factory_t {
    typedef std::type_index index_type;

    typedef std::function<
        std::unique_ptr<base_frontend_t>(const frontend_config_t&)
    > frontend_factory_type;

    struct matcher_t {
        std::function<
            index_type(const std::string&, const dynamic_t&)
        > match;

        index_type id;
    };

    struct adder_t {
        factory_t* factory;

        template<class Sink, class Formatter>
        void operator()(aux::util::metaholder<Sink, Formatter>) const {
            factory->add<Sink, Formatter>();
        }
    };

    std::unordered_map<
        index_type,
        std::unordered_map<index_type, frontend_factory_type>
    > factories;

    struct {
        std::vector<matcher_t> formatter;
        std::vector<matcher_t> sink;
    } index;

    mutable std::mutex mutex;

private:
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
        boost::mpl::is_sequence<Sink>::type::value &&
        boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        boost::mpl::for_each<
            Sink,
            aux::util::metaholder<boost::mpl::_, Formatter>
        >(adder_t { this });
    }

    template<class Sink, class Formatter>
    typename std::enable_if<
         boost::mpl::is_sequence<Sink>::type::value &&
        !boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        boost::mpl::for_each<
            Sink,
            aux::util::metaholder<boost::mpl::_, Formatter>
        >(adder_t { this });
    }

    template<class Sink, class Formatter>
    typename std::enable_if<
        !boost::mpl::is_sequence<Sink>::type::value &&
         boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        boost::mpl::for_each<
            Formatter,
            aux::util::metaholder<Sink, boost::mpl::_>
        >(adder_t { this });
    }

    template<class Sink, class Formatter>
    typename std::enable_if<
        !boost::mpl::is_sequence<Sink>::type::value &&
        !boost::mpl::is_sequence<Formatter>::type::value
    >::type
    add() {
        index.formatter.push_back(matcher_t {
            std::bind(
                &match_traits<Formatter>::type_index,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            index_type(typeid(Formatter))
        });

        index.sink.push_back(matcher_t {
            std::bind(
                &match_traits<Sink>::type_index,
                std::placeholders::_1,
                std::placeholders::_2
            ),
            index_type(typeid(Sink))
        });

        factories[typeid(Formatter)][typeid(Sink)] = std::bind(
            &create_frontend<Formatter, Sink>,
            std::placeholders::_1
        );
    }

    std::unique_ptr<base_frontend_t>
    create(const formatter_config_t& formatter, const sink_config_t& sink) const {
        return create(frontend_config_t { formatter, sink });
    }

    std::unique_ptr<base_frontend_t>
    create(const frontend_config_t& config) const {
        auto fit = std::find_if(
            index.formatter.begin(),
            index.formatter.end(),
            std::bind(
                &match,
                std::cref(config.formatter.type()),
                std::cref(config.formatter.config()),
                std::placeholders::_1
            )
        );

        auto sid = std::find_if(index.sink.begin(), index.sink.end(), [&config](const matcher_t& t) {
            return t.match(config.sink.type(), config.sink.config()) == t.id;
        });

        if (fit == index.formatter.end()) {
            throw blackhole::error_t("formatter not registered");
        }

        if (sid == index.sink.end()) {
            throw blackhole::error_t("sink not registered");
        }

        auto it = factories.find(fit->id);
        if (it == factories.end()) {
            throw blackhole::error_t("formatter not registered");
        }

        auto map2 = it->second;
        auto it2 = map2.find(sid->id);
        if (it2 == map2.end()) {
            throw blackhole::error_t("sink not registered");
        }

        return it2->second(config);
    }

    template<class Sink, class Formatter>
    bool has() const {
        std::lock_guard<std::mutex> lock(mutex);

        auto fid = std::find_if(index.formatter.begin(), index.formatter.end(), [](const matcher_t& t) {
            return t.id == std::type_index(typeid(Formatter));
        });

        auto sid = std::find_if(index.sink.begin(), index.sink.end(), [](const matcher_t& t) {
            return t.id == std::type_index(typeid(Sink));
        });

        return fid != index.formatter.end() && sid != index.sink.end();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        index.formatter.clear();
        index.sink.clear();
        factories.clear();
    }

private:
    static
    inline
    bool
    match(const std::string& type, const dynamic_t& config, const matcher_t& matcher) {
        return matcher.match(type, config) == matcher.id;
    }
};

} // namespace blackhole
