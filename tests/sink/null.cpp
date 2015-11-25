#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <blackhole/config.hpp>
#include <blackhole/config/monadic.hpp>
#include <blackhole/record.hpp>
#include <blackhole/sink/null.hpp>

namespace blackhole {
namespace testing {

using ::testing::StrictMock;

using sink::null_t;

namespace mock {

class config_t : public ::blackhole::config_t {
    typedef ::blackhole::config_t mocked_type;

public:
    MOCK_CONST_METHOD1(subscript, mocked_type*(const std::size_t&));
    MOCK_CONST_METHOD1(subscript, mocked_type*(const std::string&));

    auto operator[](const std::size_t& idx) const -> config::monadic<mocked_type> {
        return config::monadic<mocked_type>(std::unique_ptr<mocked_type>(subscript(idx)));
    }

    auto operator[](const std::string& key) const -> config::monadic<mocked_type> {
        return config::monadic<mocked_type>(std::unique_ptr<mocked_type>(subscript(key)));
    }

    MOCK_CONST_METHOD0(to_bool, bool());
    MOCK_CONST_METHOD0(to_i64, std::int64_t());
    MOCK_CONST_METHOD0(to_u64, std::uint64_t());
    MOCK_CONST_METHOD0(to_double, double());
    MOCK_CONST_METHOD0(to_string, std::string());

    MOCK_METHOD1(each, void(const each_function&));
    MOCK_METHOD1(each_map, void(const member_function&));
};

}  // namespace mock

TEST(null_t, FilterOut) {
    const string_view message("-");
    const attribute_pack pack;
    record_t record(42, message, pack);

    null_t sink;

    EXPECT_FALSE(sink.filter(record));
}

TEST(null_t, factory) {
    StrictMock<mock::config_t> config;

    // NOTE: Actually does nothing, none of mock methods should be called.
    factory<sink::null_t>::from(config);
}

}  // namespace testing
}  // namespace blackhole
