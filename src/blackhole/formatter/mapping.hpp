#pragma once

#include <functional>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/utils/unique.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace mapping {

struct actor_t {
    virtual std::string execute(const log::attribute_value_t& value) const = 0;
};

template<typename T>
struct actor_impl_t : public actor_t {
    std::function<std::string(const T&)> func;

    actor_impl_t(std::function<std::string(const T&)> func) :
        func(func)
    {}

    std::string execute(const log::attribute_value_t& value) const {
        typedef typename aux::underlying_type<T>::type underlying_type;
        return func(static_cast<T>(boost::get<underlying_type>(value)));
    }
};

class mapper_t {
    std::unordered_map<std::string, std::unique_ptr<actor_t>> actors;

public:
    template<typename T>
    void add(const std::string& key, std::function<std::string(const T&)> handler) {
        std::unique_ptr<actor_t> actor = std::make_unique<actor_impl_t<T>>(handler);
        actors[key] = std::move(actor);
    }

    std::tuple<std::string, bool> execute(const std::string& key, const log::attribute_value_t& value) const {
        auto it = actors.find(key);
        if (it != actors.end()) {
            actor_t* actor = it->second.get();
            return std::make_tuple(actor->execute(value), true);
        }
        return std::make_tuple("", false);
    }
};

} // namespace mapping

} // namespace blackhole
