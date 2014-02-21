#pragma once

#include <string>

#include <blackhole/sink/syslog.hpp>

namespace testing {

namespace mock {

namespace syslog {

class backend_t {
public:
    backend_t(const std::string&, int, int) {}

    MOCK_CONST_METHOD2(write, void(blackhole::sink::priority_t, const std::string&));
};

} // namespace syslog

} // namespace mock

} // namespace testing
