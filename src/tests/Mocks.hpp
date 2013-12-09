#pragma once

#include "Global.hpp"

//logger &log;

//log.debug("message: %s", "str",
//          keyword::verbosity=5);

////pattern="[%(timestamp)s] [%(level)s]: %(message)s%(...other)s"
////[012391232139] [debug] bla: 12313, name: "foo", ...


//LOG(log, debug, "le message: %s", "just some message")(
//    tag::groups(100500),
//    tag::http::code(404),
//    { "groups", 100500 }
//);


///* verbosity: info
// * sink: file
// *   path: [/]
// *   format: string
// *     pattern: [%(...)s] ...
// *
// * sink: tcp/udp
// *   address: 0.0.0.0
// *   port: 5000
// *   format: msgpack/json
// *
// * sink: syslog
// *   identity: blah
// *   format: string
// *     pattern: [%(...)s] ...
// */

//! Python
// log.debug("message from '%s'", "Hell", { "owner": "Satan", "email": "satan@mail.hell" })


//auto core = logging::core::instance();

using namespace blackhole;

namespace mock {

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

class frontend_t : public base_frontend_t {
public:
    MOCK_METHOD1(handle, void(const log::record_t&));
};

namespace socket {

class backend_t {
public:
    backend_t(const std::string&, std::uint16_t) {
    }

    MOCK_CONST_METHOD1(write, void(const std::string&));
};

} // namespace socket

} // namespace mock
