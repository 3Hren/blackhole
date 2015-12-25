#include <gmock/gmock.h>

namespace blackhole {
namespace config {
namespace testing {
namespace mock {

class node_t : public config::node_t {
public:
    MOCK_CONST_METHOD0(to_bool, bool());
    MOCK_CONST_METHOD0(to_sint64, std::int64_t());
    MOCK_CONST_METHOD0(to_uint64, std::uint64_t());
    MOCK_CONST_METHOD0(to_double, double());
    MOCK_CONST_METHOD0(to_string, std::string());

    MOCK_METHOD1(each, void(const each_function&));
    MOCK_METHOD1(each_map, void(const member_function&));

    MOCK_CONST_METHOD1(subscript_idx, config::node_t*(const std::size_t&));
    MOCK_CONST_METHOD1(subscript_key, config::node_t*(const std::string&));

    auto operator[](const std::size_t& idx) const -> option<config::node_t> {
        return option<config::node_t>(std::unique_ptr<config::node_t>(subscript_idx(idx)));
    }

    auto operator[](const std::string& key) const -> option<config::node_t> {
        return option<config::node_t>(std::unique_ptr<config::node_t>(subscript_key(key)));
    }
};

}  // namespace mock
}  // namespace testing
}  // namespace config
}  // namespace blackhole
