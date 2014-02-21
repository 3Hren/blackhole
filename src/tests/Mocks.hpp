#pragma once

#include "Global.hpp"

namespace testing {

namespace mock {

//class formatter_t {
//public:
//    MOCK_CONST_METHOD1(format, std::string(const log::record_t&));
//};

//class sink_t {
//public:
//    MOCK_METHOD1(consume, void(const std::string&));
//};

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
