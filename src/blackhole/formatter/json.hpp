#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/error.hpp"
#include "blackhole/formatter/base.hpp"
#include "blackhole/formatter/json/config.hpp"
#include "blackhole/record.hpp"
#include "blackhole/utils/actions/empty.hpp"
#include "blackhole/utils/nullptr.hpp"
#include "blackhole/utils/split.hpp"

namespace blackhole {

namespace formatter {

//! This class looks creppy, because of inconvenient rapidjson interface. Hope someout could refactor it.
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

    /*!
     * \brief map_config
     *        "/" -> ["message"]
     *        "/fields" -> "*"
     *        =>
     *        "specified" { "message" -> [] }
     *        "unspecified" -> ["fields"]
     */
    static config_type map_config(const boost::any& config) {
        using namespace formatter::json::map;

        std::vector<boost::any> options;
        aux::any_to(config, options);

        config_type cfg;
        aux::any_to(options.at(NEWLINE_ID), cfg.newline);
        aux::any_to(options.at(NAMING_ID), cfg.naming);

        std::unordered_map<std::string, boost::any> positioning;
        aux::any_to(options.at(POSITIONING_ID), positioning);
        for (auto it = positioning.begin(); it != positioning.end(); ++it) {
            const std::string& name = it->first;
            const boost::any& value = it->second;

            if (value.type() == typeid(std::string)) {
                handle_unspecified(name, value, cfg);
            } else if (value.type() == typeid(std::vector<std::string>)) {
                handle_specified(name, value, cfg);
            } else {
                throw blackhole::error_t("wrong configuration");
            }
        }

        return cfg;
    }

    static void handle_specified(const std::string& name, const boost::any& value, config_type& cfg) {
        std::vector<std::string> positions = aux::split(name, "/");
        std::vector<std::string> keys;
        aux::any_to(value, keys);
        for (auto it = keys.begin(); it != keys.end(); ++it) {
            const std::string& key = *it;
            cfg.positioning.specified[key] = positions;
        }
    }

    static void handle_unspecified(const std::string& name, const boost::any& value, config_type& cfg) {
        if (boost::any_cast<std::string>(value) == "*") {
            cfg.positioning.unspecified = aux::split(name, "/");
        } else {
            throw blackhole::error_t("wrong configuration");
        }
    }
};

} // namespace blackhole
