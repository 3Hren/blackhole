#pragma once

namespace testing {

namespace mock {

namespace socket {

class backend_t {
public:
    backend_t(const std::string&, std::uint16_t) {}

    MOCK_CONST_METHOD1(write, ssize_t(const std::string&));
};

class failing_backend_t : public backend_t {
public:
    failing_backend_t(const std::string& host, std::uint16_t port) :
        backend_t(host, port)
    {
        throw std::exception();
    }
};

} // namespace socket

} // namespace mock

} // namespace testing
