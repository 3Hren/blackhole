#pragma once

namespace blackhole {
inline namespace v1 {

class severity_t;

class formatter_t;
class handler_t;
class registry_t;
class sink_t;

template<typename T>
struct factory;

class factory_t;

namespace experimental {

template<typename T>
class factory;

}  // namespace experimental

namespace config {

class node_t;

}  // namespace config
}  // namespace v1
}  // namespace blackhole
