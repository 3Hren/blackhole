#pragma once

#include <blackhole/record.hpp>

namespace testing {

namespace mock {

template<typename Level>
class verbose_log_t {
public:
    typedef Level level_type;

    MOCK_CONST_METHOD1_T(open_record, blackhole::log::record_t(Level));
    MOCK_CONST_METHOD1_T(push, void(blackhole::log::record_t));
};

} // namespace mock

} // namespace testing
