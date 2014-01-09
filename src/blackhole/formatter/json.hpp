#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "blackhole/error.hpp"
#include "blackhole/formatter/base.hpp"
#include "blackhole/formatter/json/config.hpp"
#include "blackhole/record.hpp"
#include "blackhole/utils/nullptr.hpp"

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

    //!@todo: Maybe replace pointer by its reference? This can make code a bit cleaner.
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

//!@todo: move to utils/actions
struct empty_action {
    template<typename T>
    bool operator ()(const T& value) const {
        return value.empty();
    }
};

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
                if (boost::any_cast<std::string>(value) == "*") {
                    boost::split(cfg.positioning.unspecified, name, boost::is_any_of("/"));
                    cfg.positioning.unspecified.erase(std::remove_if(cfg.positioning.unspecified.begin(),
                                                                     cfg.positioning.unspecified.end(),
                                                                     empty_action()),
                                                      cfg.positioning.unspecified.end());
                } else {
                    throw blackhole::error_t("wrong configuration");
                }
            } else if (value.type() == typeid(std::vector<std::string>)) {
                std::vector<std::string> positions;
                boost::split(positions, name, boost::is_any_of("/"));
                positions.erase(std::remove_if(positions.begin(),
                                               positions.end(),
                                               empty_action()),
                                positions.end());

                std::vector<std::string> keys;
                aux::any_to(value, keys);
                for (auto key_it = keys.begin(); key_it != keys.end(); ++key_it) {
                    const std::string& key = *key_it;
                    cfg.positioning.specified[key] = positions;
                }
            } else {
                throw blackhole::error_t("wrong configuration");
            }
        }

        return cfg;
    }
};

} // namespace blackhole
