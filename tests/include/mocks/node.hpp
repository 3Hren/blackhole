#include <gmock/gmock.h>

#include <blackhole/config/node.hpp>
#include <blackhole/config/option.hpp>

namespace blackhole {
inline namespace v1 {
namespace config {
namespace testing {
namespace mock {

class node_t : public ::blackhole::config::node_t {
    typedef ::blackhole::config::node_t super;

public:
    MOCK_CONST_METHOD0(to_bool, bool());
    MOCK_CONST_METHOD0(to_sint64, std::int64_t());
    MOCK_CONST_METHOD0(to_uint64, std::uint64_t());
    MOCK_CONST_METHOD0(to_double, double());
    MOCK_CONST_METHOD0(to_string, std::string());

    MOCK_METHOD1(each, void(const each_function&));
    MOCK_METHOD1(each_map, void(const member_function&));

    MOCK_CONST_METHOD1(subscript_idx, super*(const std::size_t&));
    MOCK_CONST_METHOD1(subscript_key, super*(const std::string&));

    auto operator[](const std::size_t& idx) const -> option<super> {
        if (auto node = subscript_idx(idx)) {
            return option<super>(std::unique_ptr<super>(node));
        } else {
            return {};
        }
    }

    auto operator[](const std::string& key) const -> option<super> {
        if (auto node = subscript_key(key)) {
            return option<super>(std::unique_ptr<super>(node));
        } else {
            return {};
        }
    }
};

}  // namespace mock
}  // namespace testing
}  // namespace config
}  // namespace v1
}  // namespace blackhole

#include <blackhole/config/factory.hpp>

namespace blackhole {
inline namespace v1 {
namespace config {

template<>
class factory<testing::mock::node_t> : public factory_t {
public:
    MOCK_CONST_METHOD0(config, const node_t&());
};

}  // namespace config
}  // namespace v1
}  // namespace blackhole
