#pragma once

#include <blackhole/frontend.hpp>

namespace testing {

namespace mock {

class frontend_t : public blackhole::base_frontend_t {
public:
    MOCK_METHOD1(handle, void(const blackhole::log::record_t&));
};

} // namespace mock

} // namespace testing
