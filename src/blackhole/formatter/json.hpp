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

class json_t {
public:
    std::string format(const log::record_t& record) const {
        rapidjson::Document root;
        root.SetObject();

        json_visitor_t visitor(&root);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const std::string& name = it->first;
            const log::attribute_t& attribute = it->second;
            aux::apply_visitor(visitor, name.c_str(), attribute.value);
        }

        rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
        rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);

        root.Accept(writer);
        return std::string(buffer.GetString(), buffer.Size());
    }
};

} // namespace formatter

} // namespace blackhole
