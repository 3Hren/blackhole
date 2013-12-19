#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <json/json.h>

#include "blackhole/record.hpp"
#include "blackhole/platform.hpp"

namespace blackhole {

namespace formatter {

namespace aux {

class json_visitor : public boost::static_visitor<> {
    Json::Value& root;
public:
    json_visitor(Json::Value& root) :
        root(root)
    {}

    void operator()(const std::string& name, const std::uint32_t attribute) {
        root[name] = Json::UInt(attribute);
    }

    void operator()(const std::string& name, const std::uint64_t attribute) {
        root[name] = Json::UInt64(attribute);
    }

    void operator()(const std::string& name, const std::int32_t attribute) {
        root[name] = Json::Int(attribute);
    }

    void operator()(const std::string& name, const std::int64_t attribute) {
        root[name] = Json::Int64(attribute);
    }

    void operator()(const std::string& name, const std::string& attribute) {
        root[name] = attribute;
    }

    void operator()(const std::string& name, const std::double_t attribute) {
        root[name] = attribute;
    }

#if defined(__clang__)
    void operator()(const std::string& name, const std::time_t attribute) {
        root[name] = Json::UInt64(attribute);
    }
#endif
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

//!@todo: Try to implement logstash formatter.
