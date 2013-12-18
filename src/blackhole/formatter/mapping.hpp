#pragma once

#include <functional>
#include <string>

#include "blackhole/attribute.hpp"
#include "blackhole/utils/unique.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace mapping {

template<typename T>
struct extracter {
    std::function<std::string(const T&)> func;

    extracter(std::function<std::string(const T&)> func) :
        func(func)
    {}

    std::string operator()(const log::attribute_value_t& value) const {
        typedef typename aux::underlying_type<T>::type underlying_type;
        return func(static_cast<T>(boost::get<underlying_type>(value)));
    }
};

class mapper_t {
    typedef std::function<std::string(const log::attribute_value_t&)> mapping_t;
    std::unordered_map<std::string, mapping_t> m_mappings;

public:
    template<typename T>
    void add(const std::string& key, std::function<std::string(const T&)> handler) {
        m_mappings[key] = extracter<T>(handler);
    }

    std::tuple<std::string, bool> execute(const std::string& key, const log::attribute_value_t& value) const {
        auto it = m_mappings.find(key);
        if (it != m_mappings.end()) {
            const mapping_t& action = it->second;
            return std::make_tuple(action(value), true);
        }
        return std::make_tuple("", false);
    }
};

} // namespace mapping

} // namespace blackhole
