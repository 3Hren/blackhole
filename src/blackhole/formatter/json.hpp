#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/record.hpp"

namespace blackhole {

namespace formatter {

namespace json {

struct config_t {
    typedef std::vector<std::string> fields_t;

    bool newline;
    std::unordered_map<std::string, std::string> mapping;
    std::unordered_map<std::string, fields_t> fields;

    config_t() :
        newline(false)
    {}
};

} // namespace json

class json_visitor_t : public boost::static_visitor<> {
    rapidjson::Document* root;
    const char* name;
    const json::config_t::fields_t* nodes;
public:

    json_visitor_t(rapidjson::Document* root) :
        root(root),
        name(0),
        nodes(0)
    {}

    void bind_name(const char* name) {
        this->name = name;
    }

    void bind_nodes(const json::config_t::fields_t* nodes = nullptr) {
        this->nodes = nodes;
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    void operator ()(T value) const {
        add_member(value);
    }

    void operator ()(std::time_t value) const {
        add_member(static_cast<int64_t>(value));
    }

    void operator ()(const std::string& value) const {
        add_member(value.c_str());
    }

private:
    template<typename T>
    void add_member(const T& value) const {
        if (nodes && nodes->size()) {
            rapidjson::Value* tmp = root;
            for (auto it = nodes->begin(); it != nodes->end(); ++it) {
                const std::string& n = *it;
                if (!tmp->HasMember(n.c_str())) {
                    rapidjson::Value node;
                    node.SetObject();
                    tmp->AddMember(n.c_str(), node, root->GetAllocator());
                }
                tmp = &tmp->operator [](n.c_str());
            }
            tmp->AddMember(name, value, root->GetAllocator());
        } else {
            root->AddMember(name, value, root->GetAllocator());
        }

    }
};

namespace aux {

template<typename Visitor, typename T>
void apply_visitor(Visitor& visitor, const char* name, const T& value) {
    visitor.bind_name(name);
    boost::apply_visitor(visitor, value);
}

template<typename Visitor, typename T>
void apply_visitor(Visitor& visitor, const json::config_t::fields_t* nodes, const char* name, const T& value) {
    visitor.bind_name(name);
    visitor.bind_nodes(nodes);
    boost::apply_visitor(visitor, value);
    visitor.bind_nodes();
}

}

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
            const std::string& name = mapped(it->first);
            const log::attribute_t& attribute = it->second;
            apply_visitor(visitor, name, attribute.value);
        }

        rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
        rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);

        root.Accept(writer);
        std::string message = std::string(buffer.GetString(), buffer.Size());
        if (config.newline) {
            message.push_back('\n');
        }
        return message;
    }

private:
    void apply_visitor(json_visitor_t& visitor, const std::string& name, const log::attribute_value_t& value) const {
        auto it = config.fields.find(name);
        if (it != config.fields.end()) {
            const json::config_t::fields_t* nodes = &it->second;
            aux::apply_visitor(visitor, nodes, name.c_str(), value);
        } else {
            aux::apply_visitor(visitor, name.c_str(), value);
        }
    }

    const std::string& mapped(const std::string& name) const {
        auto it = config.mapping.find(name);
        if (it != config.mapping.end()) {
            return it->second;
        }
        return name;
    }
};

} // namespace formatter

} // namespace blackhole
