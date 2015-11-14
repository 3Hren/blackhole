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

    MOCK_METHOD1(execute, void(const record_t&));
};

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
