#pragma once

#include <gmock/gmock.h>

#include <blackhole/scope/manager.hpp>

namespace blackhole {
namespace testing {
namespace mock {
namespace scope {

class manager_t : public ::blackhole::scope::manager_t {
public:
    MOCK_CONST_METHOD0(get, ::blackhole::scope::watcher_t*());
    MOCK_METHOD1(reset, void(::blackhole::scope::watcher_t*));
};

}  // namespace scope
}  // namespace mock
}  // namespace testing
}  // namespace blackhole
