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
    typedef std::unordered_map<std::string, std::string> name_mapping_t;
    typedef std::unordered_map<std::string, fields_t> field_mapping_t;

    bool newline;
    name_mapping_t name_mapping;
    field_mapping_t field_mapping;

    config_t() :
        newline(false)
    {}
};

} // namespace json

class json_visitor_t : public boost::static_visitor<> {
    rapidjson::Document* root;
    const json::config_t::field_mapping_t& field_mapping;

    const std::string* name;
public:
    json_visitor_t(rapidjson::Document* root, const json::config_t::field_mapping_t& field_mapping) :
        root(root),
        field_mapping(field_mapping),
        name(nullptr)
    {}

    void set_name(const std::string* name) {
        this->name = name;
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
        auto it = field_mapping.find(*name);
        if (it != field_mapping.end()) {
            const json::config_t::fields_t& fields = it->second;
            if (fields.size() > 0) {
                rapidjson::Value* node = root;
                for (auto it = fields.begin(); it != fields.end(); ++it) {
                    const std::string& field = *it;
                    if (!node->HasMember(field.c_str())) {
                        rapidjson::Value child;
                        child.SetObject();
                        add_member(node, field, std::move(child));
                    }
                    node = &(*node)[field.c_str()];
                }
                add_member(node, value);
            } else {
                add_member(root, value);
            }
        } else {
            add_member(root, value);
        }
    }

    template<typename T>
    void add_member(rapidjson::Value* node, const T& value) const {
        node->AddMember(name->c_str(), value, root->GetAllocator());
    }

    template<typename T>
    void add_member(rapidjson::Value* node, const std::string& name, T&& value) const {
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

        json_visitor_t visitor(&root, config.field_mapping);
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
