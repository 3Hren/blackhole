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
class sink_t;

class registry_t;

class logger_t;
class root_logger_t;

namespace experimental {

template<typename T>
class builder;

template<typename T>
class factory;

}  // namespace experimental

namespace config {

template<typename T>
class factory;
class factory_t;

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole
