#pragma once

#include <gmock/gmock.h>

#include <blackhole/sink.hpp>

namespace blackhole {
namespace testing {
namespace mock {

class sink_t : public ::blackhole::sink_t {
public:
    sink_t();
    ~sink_t();

    MOCK_METHOD1(filter, bool(const record_t&));
    MOCK_METHOD2(emit, void(const record_t&, const string_view&));
};

}  // namespace mock
}  // namespace testing
}  // namespace blackhole
