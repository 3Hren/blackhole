#pragma once

namespace mock {

class distribution_t {
public:
    typedef std::uint64_t value_type;

    MOCK_METHOD0(next, value_type());
};

} // namespace mock
