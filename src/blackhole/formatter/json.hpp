#pragma once

#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <json/json.h>

#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

namespace aux {

class json_visitor : public boost::static_visitor<> {
    Json::Value& root;
public:
    json_visitor(Json::Value& root) :
        root(root)
    {}

    template<typename T>
    void operator ()(const std::string& name, const T& attribute) {
        root[name] = attribute;
    }

    void operator ()(const std::string& name, long attribute) {
        root[name] = static_cast<int>(attribute);
    }
};

}

class json_t {
public:
    std::string format(const log::record_t& record) const {
        Json::Value root;
        aux::json_visitor visitor(root);
        for(auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            boost::variant<std::string> name = it->first;
            const log::attribute_value_t& property = it->second.value;
            boost::apply_visitor(visitor, name, property);
        }

        Json::FastWriter writer;
        return writer.write(root);
    }
};

} // namespace formatter

} // namespace blackhole
