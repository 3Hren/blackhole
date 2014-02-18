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
    void operator ()(std::ostringstream& stream, const std::string& key, T&& value) const {
        auto it = m_mappings.find(key);
        if (it != m_mappings.end()) {
            const mapping_t& action = it->second;
            stream << action(std::forward<T>(value));
        } else {
            stream << value;
        }
    }

    template<typename T>
    boost::optional<std::string> operator ()(const std::string& key, T&& value) const {
        auto it = m_mappings.find(key);
        if (it != m_mappings.end()) {
            const mapping_t& action = it->second;
            return boost::optional<std::string>(action(std::forward<T>(value)));
        }

        return boost::optional<std::string>();
    }
};

} // namespace mapping

} // namespace blackhole
