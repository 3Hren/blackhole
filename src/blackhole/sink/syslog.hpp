#pragma once

#include "syslog.h"

#include <string>

#include "blackhole/error.hpp"

namespace blackhole {

namespace sink {

namespace backend {

class native_t {
    const std::string m_identity;
public:
    native_t(const std::string& identity, int option = LOG_PID, int facility = LOG_USER) :
        m_identity(identity)
    {
        initialize(option, facility);
    }

    ~native_t() {
        ::closelog();
    }

    void write(int priority, const char* message) {
        ::syslog(priority, "%s", message);
    }

private:
    void initialize(int option, int facility) {
        if(m_identity.empty()) {
            throw error_t("no syslog identity has been specified");
        }

        ::openlog(m_identity.c_str(), option, facility);
    }
};

} // namespace backend

template<typename Backend = backend::native_t>
class syslog_t {
    Backend m_backend;
public:
    syslog_t(const std::string& identity, int option = LOG_PID, int facility = LOG_USER) :
        m_backend(identity, option, facility)
    {}

    syslog_t(const std::string& identity, int facility) :
        m_backend(identity, LOG_PID, facility)
    {}

    void consume(int priority, const std::string& message) {
//        priority = PriorityMapper::map(priority);
        m_backend.write(priority, message.c_str());
    }
};

} // namespace sink

} // namespace blackhole
