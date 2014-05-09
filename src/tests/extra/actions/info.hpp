#pragma once

#include <sstream>

#include <boost/algorithm/string.hpp>

#include <boost/assert.hpp>

#include "../result.hpp"
#include "../response/cluster/nodes/info.hpp"

namespace elasticsearch {

namespace utils {

namespace aux {

inline void substitute(std::ostringstream&) {}

template<typename... Args>
inline void substitute(std::ostringstream& stream,
                       const std::string arg,
                       const Args&... args) {
    if (!arg.empty()) {
        stream << "/" << arg;
    }

    substitute(stream, args...);
}

} // namespace aux

template<typename... Args>
inline std::string make_path(const Args&... args) {
    std::ostringstream stream;
    aux::substitute(stream, args...);
    return stream.str();
}

} // namespace utils

namespace actions {

class nodes_info_t {
public:
    typedef result_t<response::nodes_info_t> result_type;

    enum class type_t {
        none        = 0,
        settings    = 1 << 0,
        os          = 1 << 1,
        process     = 1 << 2,
        jvm         = 1 << 3,
        thread_pool = 1 << 4,
        network     = 1 << 5,
        transport   = 1 << 6,
        http        = 1 << 7,
        plugins     = 1 << 8,
        all         = settings | os | process | jvm | thread_pool |
                      network | transport | http | plugins
    };

private:
    std::string type;
    std::string nodes;

public:
    nodes_info_t(type_t type = type_t::none) :
        type(map_type(type))
    {}

    std::string path() const {
        return utils::make_path("_nodes", nodes, type);
    }

private:
    static std::string map_type(type_t type) {
        switch (type) {
        case type_t::none:
            return "none";
        case type_t::settings:
            return "settings";
        case type_t::os:
            return "os";
        case type_t::process:
            return "process";
        case type_t::jvm:
            return "jvm";
        case type_t::thread_pool:
            return "thread_pool";
        case type_t::network:
            return "network";
        case type_t::transport:
            return "transport";
        case type_t::http:
            return "http";
        case type_t::plugins:
            return "plugins";
        case type_t::all:
            return "_all";
        default:
            BOOST_ASSERT(false);
        }
    }
};

} // namespace actions

} // namespace elasticsearch
