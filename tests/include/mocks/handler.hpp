#pragma once

#include <gmock/gmock.h>

#include <blackhole/handler.hpp>

namespace blackhole {
namespace testing {
namespace mock {

class handler_t : public ::blackhole::handler_t {
public:
    handler_t();
    ~handler_t();

    MOCK_METHOD1(handle, void(const record_t&));

    auto set(std::unique_ptr<formatter_t>) -> void {}
    auto add(std::unique_ptr<sink_t>) -> void {}
};

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
