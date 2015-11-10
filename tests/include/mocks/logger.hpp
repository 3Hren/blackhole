#pragma once

#include <gmock/gmock.h>

#include <blackhole/logger.hpp>

namespace blackhole {
namespace testing {
namespace mock {

class logger_t : public ::blackhole::logger_t {
public:
    logger_t();
    ~logger_t();

    MOCK_CONST_METHOD2(log, void(int, string_view));
    MOCK_CONST_METHOD3(log, void(int, string_view, attribute_pack&));
    MOCK_CONST_METHOD4(log, void(int, string_view, attribute_pack&, const format_t&));
};

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
