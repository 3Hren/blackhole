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
    typedef std::vector<std::string> hierarchy_t;
    typedef std::unordered_map<std::string, std::string> name_mapping_t;
    typedef std::unordered_map<std::string, hierarchy_t> field_hierarchy_t;

    bool newline;
    name_mapping_t name_mapping;
    field_hierarchy_t field_hierarchy;

    config_t() :
        newline(false)
    {}
};

} // namespace json

//! This class looks creppy, because of inconvenient rapidjson interface.
class json_visitor_t : public boost::static_visitor<> {
    rapidjson::Document* root;
    const json::config_t::field_hierarchy_t& field_hierarchy;

    const std::string* name;
public:
    json_visitor_t(rapidjson::Document* root, const json::config_t::field_hierarchy_t& field_hierarchy) :
        root(root),
        field_hierarchy(field_hierarchy),
        name(nullptr)
    {}

    void set_name(const std::string* name) {
        this->name = name;
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    void operator ()(T value) const {
        apply(value);
    }

    void operator ()(std::time_t value) const {
        apply(static_cast<int64_t>(value));
    }

    void operator ()(const std::string& value) const {
        apply(value.c_str());
    }

private:
    template<typename T>
    void apply(const T& value) const {
        auto it = field_hierarchy.find(*name);
        if (it != field_hierarchy.end()) {
            const json::config_t::hierarchy_t& hierarchy = it->second;
            if (hierarchy.size() > 0) {
                build_hierarchy(hierarchy, value);
            } else {
                add_member(root, *name, value);
            }
        } else {
            add_member(root, *name, value);
        }
    }

    template<typename T>
    void build_hierarchy(const json::config_t::hierarchy_t& hierarchy, const T& value) const {
        rapidjson::Value* node = root;
        for (auto it = hierarchy.begin(); it != hierarchy.end(); ++it) {
            const std::string& name = *it;
            if (!node->HasMember(name.c_str())) {
                node = add_child(node, name);
            } else {
                node = get_child(node, name);
            }
        }
        add_member(node, *name, value);
    }

    rapidjson::Value* add_child(rapidjson::Value* node, const std::string& name) const {
        rapidjson::Value child;
        child.SetObject();
        add_member(node, name, std::move(child));
        return get_child(node, name);
    }

    rapidjson::Value* get_child(rapidjson::Value* node, const std::string& name) const {
        return &(*node)[name.c_str()];
    }

    template<typename T>
    void add_member(rapidjson::Value* node, const std::string& name, const T& value) const {
        node->AddMember(name.c_str(), value, root->GetAllocator());
    }

    void add_member(rapidjson::Value* node, const std::string& name, rapidjson::Value&& value) const {
        node->AddMember(name.c_str(), value, root->GetAllocator());
    }
};

namespace aux {

template<typename Visitor, typename T>
void apply_visitor(Visitor& visitor, const std::string* name, const T& value) {
    visitor.set_name(name);
    boost::apply_visitor(visitor, value);
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

        json_visitor_t visitor(&root, config.field_hierarchy);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const std::string& name = mapped(it->first);
            const log::attribute_t& attribute = it->second;
            aux::apply_visitor(visitor, &name, attribute.value);
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
    const std::string& mapped(const std::string& name) const {
        auto it = config.name_mapping.find(name);
        if (it != config.name_mapping.end()) {
            return it->second;
        }
        return name;
    }
};

} // namespace formatter

} // namespace blackhole
