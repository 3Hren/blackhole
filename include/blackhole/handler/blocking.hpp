#pragma once

#include <memory>
#include <vector>

#include "blackhole/handler.hpp"

namespace blackhole {
inline namespace v1 {

template<typename>
struct factory;

}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole

namespace blackhole {
inline namespace v1 {
namespace handler {

class blocking_t : public handler_t {
    std::unique_ptr<formatter_t> formatter;
    std::vector<std::unique_ptr<sink_t>> sinks;

public:
    auto handle(const record_t& record) -> void;

    auto set(std::unique_ptr<formatter_t> formatter) -> void;
    auto add(std::unique_ptr<sink_t> sink) -> void;
};

}  // namespace handler

template<>
struct factory<handler::blocking_t> {
    static auto type() -> const char*;
    static auto from(const config::node_t& config) -> handler::blocking_t;
};

}  // namespace v1
}  // namespace blackhole
