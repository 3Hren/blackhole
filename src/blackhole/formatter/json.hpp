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
    rapidjson::Document &root;
public:
    const char* name;

    json_visitor_t(rapidjson::Document& root) :
        root(root)
    {}

    template<typename T>
    void operator ()(T value) {
        root.AddMember(name, value, root.GetAllocator());
    }

    void operator ()(std::time_t value) {
        root.AddMember(name, static_cast<int64_t>(value), root.GetAllocator());
    }

    void operator ()(const std::string& value) {
        root.AddMember(name, value.c_str(), root.GetAllocator());
    }
};

class json_t {
public:
    std::string format(const log::record_t& record) const {
        rapidjson::Document root;
        root.SetObject();

        json_visitor_t visitor(root);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const log::attribute_t& attribute = it->second;
            visitor.name = it->first.c_str();
            boost::apply_visitor(visitor, attribute.value);
        }

        rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
        rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);

        root.Accept(writer);
        return std::string(buffer.GetString(), buffer.Size());
    }
};

} // namespace formatter

} // namespace blackhole

//!@todo: Find normal json builder.
//!@todo: Try to implement logstash formatter.
