#pragma once

#include <cstring>

#include "global.hpp"

namespace testing {

namespace mock {

class timer_t {
public:
    MOCK_CONST_METHOD0(current, std::time_t());
};

class time_picker_t {
public:
    time_picker_t() {
        std::tm timeinfo;
        std::memset(&timeinfo, 0, sizeof(timeinfo));
        ON_CALL(*this, now())
                .WillByDefault(Return(timeinfo));
    }

    MOCK_CONST_METHOD0(now, std::tm());
};

} // namespace mock

} // namespace testing
