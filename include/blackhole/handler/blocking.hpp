#pragma once

#include <memory>
#include <vector>

#include "blackhole/handler.hpp"

namespace blackhole {

class config_t;

template<typename>
struct factory;

namespace handler {

class blocking_t : public handler_t {
    std::unique_ptr<formatter_t> formatter;
    std::vector<std::unique_ptr<sink_t>> sinks;

public:
    auto execute(const record_t& record) -> void;

    auto set(std::unique_ptr<formatter_t> formatter) -> void;
    auto add(std::unique_ptr<sink_t> sink) -> void;
};

}  // namespace handler

template<>
struct factory<handler::blocking_t> {
    static auto type() -> const char*;
    static auto from(const config_t& config) -> handler::blocking_t;
};

}  // namespace blackhole
