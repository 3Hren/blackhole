#pragma once

#include <string>
#include <type_traits>

#include "blackhole/sink/elasticsearch/result.hpp"
#include "blackhole/sink/elasticsearch/response/bulk_write.hpp"
#include "blackhole/sink/elasticsearch/request/method.hpp"
#include "blackhole/sink/elasticsearch/request/utils.hpp"

namespace elasticsearch {

namespace actions {

class bulk_write_t {
public:
    typedef response::bulk_write_t response_type;
    typedef result_t<response_type>::type result_type;

    typedef std::integral_constant<
        request::method_t,
        request::method_t::post
    > method;

    static const char* name() {
        return "bulk_write";
    }

private:
    const std::string index;
    const std::string type;
    const std::string data;

public:
    bulk_write_t(const std::string& index,
                 const std::string& type,
                 std::vector<std::string>&& bulk) :
        index(index),
        type(type),
        data(prepare(std::move(bulk)))
    {}

    std::string path() const {
        return utils::make_path(index, type, "_bulk");
    }

    std::string body() const {
        return data;
    }

private:
    static std::string prepare(std::vector<std::string>&& bulk) {
        static std::string ACTION = "{\"index\":{}}\n";

        std::string::size_type hint = bulk.size() * ACTION.size();
        for (auto it = bulk.begin(); it != bulk.end(); ++it) {
            hint += it->size() + 1;
        }

        std::string result;
        result.reserve(hint);

        for (auto it = bulk.begin(); it != bulk.end(); ++it) {
            result.append(ACTION);
            result.append(*it);
            result.push_back('\n');
        }

        return result;
    }
};

} // namespace actions

} // namespace elasticsearch
