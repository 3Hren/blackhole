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

template<typename Level>
class formatter_factory_t {
    template<typename T>
    struct traits {
        typedef std::unique_ptr<base_frontend_t> return_type;
        typedef std::function<return_type(const formatter_config_t&, std::unique_ptr<T>)> function_type;
        typedef return_type(*raw_function_type)(const formatter_config_t&, std::unique_ptr<T>);
    };

    struct factory_keeper_t {
        std::unordered_map<std::string, boost::any> factories;

        template<typename T>
        void add(typename traits<T>::function_type fn) {
            factories[T::name()] = fn;
        }

        template<typename T, typename Formatter>
        void add() {
            typedef typename traits<T>::raw_function_type raw_function_type;
            typedef typename traits<T>::function_type function_type;
            raw_function_type overloaded = static_cast<raw_function_type>(&factory_t<Level>::template create<Formatter>);
            factories[T::name()] = static_cast<function_type>(overloaded);
        }

        template<typename T>
        typename traits<T>::function_type get() const {
            boost::any any = factories.at(T::name());
            return boost::any_cast<typename traits<T>::function_type>(any);
        }
    };

    mutable std::mutex mutex;
    std::unordered_map<std::string, factory_keeper_t> factories;
public:
    static formatter_factory_t<Level>& instance() {
        static formatter_factory_t<Level> self;
        return self;
    }

    template<typename Sink, typename Formatter>
    void add() {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = factories.find(Formatter::name());
        if (it != factories.end()) {
            factory_keeper_t& keeper = it->second;
            keeper.template add<Sink, Formatter>();
        } else {
            factory_keeper_t keeper;
            keeper.template add<Sink, Formatter>();
            factories[Formatter::name()] = keeper;
        }
    }

    template<typename Sink>
    typename traits<Sink>::return_type
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) const {
        typedef typename traits<Sink>::return_type return_type;

        try {
            std::lock_guard<std::mutex> lock(mutex);
            const factory_keeper_t& keeper = factories.at(formatter_config.type);
            auto factory = keeper.template get<Sink>();
            return factory(formatter_config, std::move(sink));
        } catch (const std::exception& err) {
            throw error_t("there are no registered formatter '%s' for sink '%s", formatter_config.type, Sink::name());
        }

        return return_type();
    }

private:
    formatter_factory_t() {}
};

} // namespace blackhole
