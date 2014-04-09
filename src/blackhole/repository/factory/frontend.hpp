#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/any.hpp>

#include "blackhole/error.hpp"
#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
#include "blackhole/repository/factory/frontend/keeper.hpp"
#include "blackhole/repository/factory/frontend/traits.hpp"

namespace blackhole {

class frontend_factory_t {
    mutable std::mutex mutex;
    std::unordered_map<std::string, function_keeper_t> factories;
public:
    template<class Sink, class Formatter>
    void add() {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = factories.find(Formatter::name());
        if (it != factories.end()) {
            function_keeper_t& keeper = it->second;
            keeper.add<Sink, Formatter>();
        } else {
            function_keeper_t keeper;
            keeper.add<Sink, Formatter>();
            factories[Formatter::name()] = keeper;
        }
    }

    template<class Sink, class Formatter>
    bool has() const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = factories.find(Formatter::name());
        return it != factories.end() && it->second.template has<Sink>();
    }

    template<class Sink>
    typename frontend::traits<Sink>::return_type
    create(const formatter_config_t& formatter_config, std::unique_ptr<Sink> sink) const {
        typedef typename frontend::traits<Sink>::return_type return_type;

        try {
            std::lock_guard<std::mutex> lock(mutex);
            const function_keeper_t& keeper = factories.at(formatter_config.type);
            auto factory = keeper.get<Sink>();
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
