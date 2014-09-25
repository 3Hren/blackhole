#pragma once

#include <cstring>

#include "global.hpp"

namespace testing {

namespace mock {

struct timer_t {
    static void mock(std::string value) {
        std::tm tm;
        std::memset(&tm, 0, sizeof(tm));
        strptime(value.c_str(), "%Y%m%d", &tm);
        timer_t::time = timegm(&tm);
    }

    static std::time_t current() {
        return time;
    }

private:
    static std::time_t time;
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
