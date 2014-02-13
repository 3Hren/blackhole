#pragma once

#include "blackhole/sink/files/rotation/watcher/config.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

namespace watcher {

template<class... Watchers>
struct watcher_set : public Watchers... {
    static const char* name() {
        return "set";
    }

    watcher_set(const config_t<watcher_set<Watchers...>>& config) :
        Watchers(config)...
    {}

    template<class Backend>
    bool operator()(Backend& backend, const std::string& message) const {
        return invoke<Backend, Watchers...>(backend, message);
    }

private:
    template<class Backend, class T, class... Args>
    bool invoke(Backend& backend, const std::string& message) const {
        return T::operator()(backend, message) || invoke<Backend, Args...>(backend, message);
    }

    template<class Backend>
    bool invoke(Backend&, const std::string&) const {
        return false;
    }
};

} // namespace watcher

} // namespace rotation

} // namespace sink

} // namespace blackhole
