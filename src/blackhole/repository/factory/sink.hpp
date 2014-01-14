#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "blackhole/repository/config/formatter.hpp"
#include "blackhole/repository/config/sink.hpp"
#include "blackhole/repository/factory/factory.hpp"

namespace blackhole {

template<typename Level>
class sink_factory_t {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef return_type(*factory_type)(const formatter_config_t&, const sink_config_t&);

    mutable std::mutex mutex;
    std::unordered_map<std::string, factory_type> sinks;
public:
    static sink_factory_t<Level>& instance() {
        static sink_factory_t<Level> self;
        return self;
    }

    template<typename T>
    void add() {
        std::lock_guard<std::mutex> lock(mutex);
        sinks[T::name()] = static_cast<factory_type>(&factory_t<Level>::template create<T>);
    }

    return_type create(const formatter_config_t& formatter_config, const sink_config_t& sink_config) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = sinks.find(sink_config.type);
        if (it != sinks.end()) {
            return it->second(formatter_config, sink_config);
        }

        return return_type();
    }

private:
    sink_factory_t() {}
};

} // namespace blackhole
