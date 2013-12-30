#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/formatter/base.hpp"
#include "blackhole/formatter/json/config.hpp"
#include "blackhole/record.hpp"
#include "blackhole/utils/nullptr.hpp"

namespace blackhole {

namespace formatter {

//! This class looks creppy, because of inconvenient rapidjson interface.
class json_visitor_t : public boost::static_visitor<> {
    rapidjson::Document* root;
    const json::map::positioning_t& positioning;

    // There is no other way to pass additional argument when invoking `apply_visitor` except
    // explicit setting it every iteration.
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
                add_positional(positions, *name, value);
            } else {
                add_member(root, *name, value);
            }
        } else if (positioning.unspecified.size() > 0) {
            add_positional(positioning.unspecified, *name, value);
        } else {
            add_member(root, *name, value);
        }
    }

    template<typename T>
    void add_positional(const json::map::positioning_t::positions_t& positions, const std::string& name, const T& value) const {
        rapidjson::Value* node = root;
        for (auto it = positions.begin(); it != positions.end(); ++it) {
            const std::string& position = *it;
            if (!node->HasMember(position.c_str())) {
                node = add_child(node, position);
            } else {
                node = get_child(node, position);
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

} // namespace aux

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

template<>
struct factory_traits<formatter::json_t> {
    typedef formatter::json_t::config_type config_type;

    static const int NEWLINE_ID = 0;
    static const int NAMING_ID = 1;
    static const int POSITIONING_ID = 2;

    static const int SPECIFIED_ID = 0;
    static const int UNSPECIFIED_ID = 1;

    static config_type map_config(const boost::any& config) {
        using namespace formatter::json::map;

        std::vector<boost::any> options = boost::any_cast<std::vector<boost::any>>(config);
        config_type cfg;
        cfg.newline = boost::any_cast<bool>(options.at(NEWLINE_ID));
        cfg.naming = boost::any_cast<naming_t>(options.at(NAMING_ID));
        std::vector<boost::any> positioning = boost::any_cast<std::vector<boost::any>>(options.at(POSITIONING_ID));

        cfg.positioning.specified = boost::any_cast<decltype(cfg.positioning.specified)>(positioning.at(SPECIFIED_ID));
        cfg.positioning.unspecified = boost::any_cast<decltype(cfg.positioning.unspecified)>(positioning.at(UNSPECIFIED_ID));
        return cfg;
    }
};

} // namespace blackhole
