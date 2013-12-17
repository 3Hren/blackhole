#pragma once

#include "Global.hpp"

using namespace blackhole;

namespace mock {

class frontend_t : public base_frontend_t {
public:
    MOCK_METHOD1(handle, void(const log::record_t&));
};

namespace files {

class backend_t {
public:
    backend_t(const std::string&) {
        ON_CALL(*this, opened()).
                WillByDefault(Return(true));
    }

    MOCK_CONST_METHOD0(opened, bool());
    MOCK_CONST_METHOD0(path, std::string());

    MOCK_METHOD0(open, bool());
    MOCK_METHOD1(write, void(const std::string&));
};

} // namespace files

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

namespace syslog {

class backend_t {
public:
    backend_t(const std::string&, int, int) {}

    MOCK_CONST_METHOD2(write, void(sink::priority_t, const std::string&));
};

} // namespace syslog

} // namespace mock
