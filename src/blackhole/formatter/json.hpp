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

namespace map {

typedef std::unordered_map<std::string, std::string> naming_t;

struct positioning_t {
    typedef std::vector<std::string> positions_t;

    std::unordered_map<std::string, positions_t> specified;
    positions_t unspecified;
};

} // namespace map

struct config_t {
    bool newline;
    map::naming_t naming;
    map::positioning_t positioning;

    config_t() :
        newline(false)
    {}
};

} // namespace json

//! This class looks creppy, because of inconvenient rapidjson interface.
class json_visitor_t : public boost::static_visitor<> {
    rapidjson::Document* root;
    const json::map::positioning_t& positioning;

    const std::string* name;
public:
    json_visitor_t(rapidjson::Document* root, const json::map::positioning_t& positioning) :
        root(root),
        positioning(positioning),
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
        auto it = positioning.specified.find(*name);
        if (it != positioning.specified.end()) {
            const json::map::positioning_t::positions_t& positions = it->second;
            if (positions.size() > 0) {
                build_hierarchy(positions, *name, value);
            } else {
                add_member(root, *name, value);
            }
        } else if (positioning.unspecified.size() > 0) {
            build_hierarchy(positioning.unspecified, *name, value);
        } else {
            add_member(root, *name, value);
        }
    }

    template<typename T>
    void build_hierarchy(const json::map::positioning_t::positions_t& positions, const std::string& name, const T& value) const {
        rapidjson::Value* node = root;
        for (auto it = positions.begin(); it != positions.end(); ++it) {
            const std::string& current_name = *it;
            if (!node->HasMember(current_name.c_str())) {
                node = add_child(node, current_name);
            } else {
                node = get_child(node, current_name);
            }
        }

        add_member(node, name, value);
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
void apply_visitor(Visitor& visitor, const std::string& name, const T& value) {
    visitor.set_name(&name);
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

        json_visitor_t visitor(&root, config.positioning);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const std::string& name = mapped(it->first);
            const log::attribute_t& attribute = it->second;
            aux::apply_visitor(visitor, name, attribute.value);
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
        auto it = config.naming.find(name);
        if (it != config.naming.end()) {
            return it->second;
        }
        return name;
    }
};

} // namespace formatter

} // namespace blackhole
