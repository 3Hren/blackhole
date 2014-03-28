#pragma once

#include <functional>
#include <string>

#include <boost/format.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/utils/unique.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace mapping {

template<typename T>
struct extracter {
    typedef std::function<void(aux::attachable_ostringstream&, const T&)> function_type;
    function_type func;

    extracter(std::function<void(aux::attachable_ostringstream&, const T&)> func) :
        func(func)
    {}

    void operator()(aux::attachable_ostringstream& stream, const log::attribute_value_t& value) const {
        typedef typename aux::underlying_type<T>::type underlying_type;
        func(stream, static_cast<T>(boost::get<underlying_type>(value)));
    }
};

class value_t {
    typedef std::function<void(aux::attachable_ostringstream&, const log::attribute_value_t&)> mapping_t;
    std::unordered_map<std::string, mapping_t> m_mappings;

public:
    template<typename T>
    void add(const std::string& key, typename extracter<T>::function_type handler) {
        m_mappings[key] = extracter<T>(handler);
    }

    template<typename Keyword>
    void add(typename extracter<typename Keyword::type>::function_type handler) {
        add<typename Keyword::type>(Keyword::name(), handler);
    }

    template<typename T>
    void operator ()(aux::attachable_ostringstream& stream, const std::string& key, T&& value) const {
        auto it = m_mappings.find(key);
        if (it != m_mappings.end()) {
            const mapping_t& action = it->second;
            action(stream, std::forward<T>(value));
        } else {
            stream << value;
        }
    }

    template<typename T>
    boost::optional<std::string> operator ()(const std::string& key, T&& value) const {
        auto it = m_mappings.find(key);
        if (it != m_mappings.end()) {
            const mapping_t& action = it->second;
            std::string buffer;
            aux::attachable_ostringstream stream;
            stream.attach(buffer);
            action(stream, std::forward<T>(value));
            stream.flush();
            return boost::optional<std::string>(buffer);
        }

        return boost::optional<std::string>();
    }
};

} // namespace mapping

} // namespace blackhole
