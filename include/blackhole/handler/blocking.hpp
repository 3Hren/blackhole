#pragma once

#include <memory>
#include <vector>

#include "blackhole/handler.hpp"

namespace blackhole {

class formatter_t;
class sink_t;

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
}  // namespace blackhole
