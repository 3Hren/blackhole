#pragma once

namespace blackhole {
inline namespace v1 {

struct deleter_t {
    template<typename T>
    auto operator()(T* value) -> void;
};

class record_t;
class writer_t;
class severity_t;

class formatter_t;
class handler_t;
class filter_t;
class sink_t;

class registry_t;

class logger_t;
class root_logger_t;

template<typename T>
class builder;

template<typename T>
class factory;

namespace config {

class node_t;
class factory_t;

/// Must implement `construct` method, which accepts anything required for factory construction and
/// returns an `std::unique_ptr<factory_t>`.
template<typename T>
class factory_traits;

}  // namespace config

class termcolor_t;

}  // namespace v1
}  // namespace blackhole
