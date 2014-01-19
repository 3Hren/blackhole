#pragma once

#include <string>

#include <boost/variant.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/expression/helper.hpp"
#include "blackhole/filter.hpp"
#include "blackhole/utils/underlying.hpp"

namespace blackhole {

namespace expression {

template<typename T>
class has_attribute_visitor : public boost::static_visitor<bool>{
public:
    bool operator ()(const T&) const {
        return true;
    }

    template<typename Other>
    bool operator ()(const Other&) const {
        return false;
    }
};

template<typename T>
struct has_attr_action_t : public aux::LogicMixin<has_attr_action_t<T>> {
    const std::string name;

    has_attr_action_t(const std::string& name) : name(name) {}

    bool operator()(const log::attributes_t& attributes) const {
        auto it = attributes.find(name);
        if (it == attributes.end()) {
            return false;
        }
        const log::attribute_t& attribute = it->second;
        typedef typename blackhole::aux::underlying_type<T>::type underlying_type;
        return boost::apply_visitor(has_attribute_visitor<underlying_type>(), attribute.value);
    }
};

// For dynamic attributes.
template<typename T>
has_attr_action_t<T> has_attr(const std::string& name) {
    return has_attr_action_t<T>(name);
}

// For static attributes.
template<typename T>
has_attr_action_t<typename T::type> has_attr(const T&) {
    return has_attr<typename T::type>(std::string(T::name()));
}

} // namespace expression

} // namespace blackhole
