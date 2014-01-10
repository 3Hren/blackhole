#pragma once

#include <functional>
#include <string>

#include <boost/format.hpp>

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

    std::string operator()(const T& value) const {
        return func(value);
    }

    std::string operator()(T value) const {
        return func(value);
    }

    std::string operator()(const log::attribute_value_t& value) const {
        typedef typename aux::underlying_type<T>::type underlying_type;
        return func(static_cast<T>(boost::get<underlying_type>(value)));
    }
};

class value_t {
    typedef std::function<std::string(const log::attribute_value_t&)> mapping_t;
    std::unordered_map<std::string, mapping_t> m_mappings;

public:
    template<typename T>
    void add(const std::string& key, std::function<std::string(const T&)> handler) {
        m_mappings[key] = extracter<T>(handler);
    }

    template<typename Keyword>
    void add(std::function<std::string(const typename Keyword::type&)> handler) {
        add(Keyword::name(), handler);
    }

    template<typename T>
    boost::optional<std::string> execute(const std::string& key, T&& value) const {
        boost::optional<std::string> result;
        auto it = m_mappings.find(key);
        if (it != m_mappings.end()) {
            const mapping_t& action = it->second;
            result = std::move(action(std::forward<T>(value)));
        }

        return result;
    }
};

inline void apply(const value_t& mapper, const std::string& key, const log::attribute_t& attribute, boost::format* format) {
    auto result = mapper.execute(key, attribute.value);
    if (result.is_initialized()) {
        (*format) % result.get();
    } else {
        (*format) % attribute.value;
    }
}

} // namespace mapping

} // namespace blackhole
