#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/cpp17/string_view.hpp>
#include <blackhole/severity.hpp>
#include <blackhole/sink/syslog.hpp>

#include <blackhole/detail/procname.hpp>

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace {

using ::testing::Return;
using ::testing::StrictMock;

class mock_backend_t : public syslog::backend_t {
public:
    virtual auto close() noexcept -> void override { close_(); }

    MOCK_METHOD0(open, void());
    MOCK_METHOD0(close_, void());
    MOCK_METHOD2(write, void(int, const string_view&));
};
//
// TEST(syslog_t, Type) {
//     EXPECT_STREQ("syslog", factory<syslog_t>::type());
// }

// TEST(syslog_t, RAII) {
//     std::unique_ptr<mock_backend_t> backend(new mock_backend_t());
//
//     EXPECT_CALL(*backend, open())
//         .Times(1);
//
//     EXPECT_CALL(*backend, close_())
//         .Times(1);
//
//     factory<syslog_t>::construct(std::move(backend), [](severity_t) -> int { return 0; });
// }

}  // namespace
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
