#pragma once

#include <gmock/gmock.h>

#include <blackhole/registry.hpp>

namespace blackhole {
inline namespace v1 {

class mock_registry_t : public registry_t {
public:
    MOCK_CONST_METHOD1(sink, registry_t::sink_factory(const std::string& type));
    MOCK_CONST_METHOD1(handler, registry_t::handler_factory(const std::string& type));
    MOCK_CONST_METHOD1(formatter, registry_t::formatter_factory(const std::string& type));
    MOCK_METHOD1(add, void(std::shared_ptr<factory<sink_t>> factory));
    MOCK_METHOD1(add, void(std::shared_ptr<factory<handler_t>> factory));
    MOCK_METHOD1(add, void(std::shared_ptr<factory<formatter_t>> factory));
};

}  // namespace v1
}  // namespace blackhole
