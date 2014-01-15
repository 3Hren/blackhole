#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "blackhole/error.hpp"
#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
#include "blackhole/repository/factory/factory.hpp"

namespace blackhole {

template<typename T>
struct traits {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef return_type(*function_type)(const formatter_config_t&, std::unique_ptr<T>);
};

//! Keeps frontend factory functions using type erasure idiom.
//! For each desired Formatter-Sink pair a function created and registered.
//! Later that function can be extracted by Sink type parameter.
template<typename Level>
struct function_keeper_t {
    std::unordered_map<std::string, boost::any> functions;

    template<typename Sink, typename Formatter>
    void add() {
        typedef typename traits<Sink>::function_type function_type;
        function_type function = static_cast<function_type>(&factory_t<Level>::template create<Formatter>);
        functions[Sink::name()] = function;
    }

    template<typename Sink>
    bool has() const {
        return functions.find(Sink::name()) != functions.end();
    }

    template<typename Sink>
    typename traits<Sink>::function_type get() const {
        boost::any any = functions.at(Sink::name());
        return boost::any_cast<typename traits<Sink>::function_type>(any);
    }
};

template<typename Level>
class frontend_factory_t {
    mutable std::mutex mutex;
    std::unordered_map<std::string, function_keeper_t<Level>> factories;
public:
    template<typename Sink, typename Formatter>
    void add() {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = factories.find(Formatter::name());
        if (it != factories.end()) {
            function_keeper_t<Level>& keeper = it->second;
            keeper.template add<Sink, Formatter>();
        } else {
            function_keeper_t<Level> keeper;
            keeper.template add<Sink, Formatter>();
            factories[Formatter::name()] = keeper;
        }
    }

    template<typename Sink, typename Formatter>
    bool has() const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = factories.find(Formatter::name());
        return it != factories.end() && it->second.template has<Sink>();
    }

    template<typename Sink>
    typename traits<Sink>::return_type
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) const {
        typedef typename traits<Sink>::return_type return_type;

        try {
            std::lock_guard<std::mutex> lock(mutex);
            const function_keeper_t<Level>& keeper = factories.at(formatter_config.type);
            auto factory = keeper.template get<Sink>();
            return factory(formatter_config, std::move(sink));
        } catch (const std::exception&) {
            throw error_t("there are no registered formatter '%s' for sink '%s", formatter_config.type, Sink::name());
        }

        return return_type();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        factories.clear();
    }
};

} // namespace blackhole
