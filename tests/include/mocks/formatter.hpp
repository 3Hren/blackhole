#pragma once

#include <gmock/gmock.h>

#include <blackhole/formatter.hpp>

namespace blackhole {
namespace testing {
namespace mock {

class formatter_t : public ::blackhole::formatter_t {
public:
    formatter_t();
    ~formatter_t();

    MOCK_METHOD2(format, void(const record_t&, writer_t&));
};

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
