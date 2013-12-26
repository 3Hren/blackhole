#pragma once

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

class json_visitor_t : public boost::static_visitor<> {
    const char* name;
    rapidjson::Document* root;
public:

    json_visitor_t(rapidjson::Document* root) :
        root(root)
    {}

    inline void bind(const char* name) {
        this->name = name;
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    void operator ()(T value) const {
        root->AddMember(name, value, root->GetAllocator());
    }

    void operator ()(std::time_t value) const {
        root->AddMember(name, static_cast<int64_t>(value), root->GetAllocator());
    }

    void operator ()(const std::string& value) const {
        root->AddMember(name, value.c_str(), root->GetAllocator());
    }
};

namespace aux {

template<typename Visitor, typename T>
void apply_visitor(Visitor& visitor, const char* name, const T& value) {
    visitor.bind(name);
    boost::apply_visitor(visitor, value);
}

}

namespace json {

struct config_t {
    bool newline;
    std::unordered_map<std::string, std::string> mapping;
    std::unordered_map<std::string, std::vector<std::string>> tree;

    config_t() :
        newline(false)
    {}
};

} // namespace json

class json_t {
    const json::config_t config;
public:
    typedef json::config_t config_type;

    json_t(const json::config_t& config = json::config_t()) :
        config(config)
    {}

    std::string format(const log::record_t& record) const {
        rapidjson::Document root;
        root.SetObject();

        json_visitor_t visitor(&root);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const std::string& name = it->first;
            const log::attribute_t& attribute = it->second;
            apply_visitor(visitor, name, attribute.value);
        }

        rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
        rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);

        root.Accept(writer);
        return std::string(buffer.GetString(), buffer.Size());
    }

private:
    void apply_visitor(json_visitor_t& visitor, const std::string& name, const log::attribute_value_t& value) const {
        auto it = config.mapping.find(name);
        if (it != config.mapping.end()) {
            aux::apply_visitor(visitor, it->second.c_str(), value);
        } else {
            aux::apply_visitor(visitor, name.c_str(), value);
        }
    }
};

} // namespace formatter

} // namespace blackhole
