#pragma once

#include "syslog.h"

#include <string>

#include "blackhole/error.hpp"
#include "blackhole/factory.hpp"

namespace blackhole {

namespace sink {

enum class priority_t : unsigned {
    emerg   = LOG_EMERG,
    alert   = LOG_ALERT,
    crit    = LOG_CRIT,
    err     = LOG_ERR,
    warning = LOG_WARNING,
    notice  = LOG_NOTICE,
    info    = LOG_INFO,
    debug   = LOG_DEBUG
};

template<typename Level>
struct priority_traits;

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

    void write(priority_t priority, const std::string& message) {
        ::syslog(static_cast<unsigned>(priority), "%s", message.c_str());
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

namespace syslog {

struct config_t {
    std::string identity;
    int option;
    int facility;

    config_t() {}

    config_t(const std::string& identity, int option = LOG_PID, int facility = LOG_USER) :
        identity(identity),
        option(option),
        facility(facility)
    {}

    config_t(const std::string& identity, int facility) :
        identity(identity),
        option(LOG_PID),
        facility(facility)
    {}
};

} // namespace syslog

template<typename Level, typename Backend = backend::native_t>
class syslog_t {
    Backend m_backend;

public:
    typedef syslog::config_t config_type;

    static const char* name() {
        return "syslog";
    }

    syslog_t(const config_type& config) :
        m_backend(config.identity, config.option, config.facility)
    {
        static_assert(std::is_enum<Level>::value, "level type must be enum");
    }

    syslog_t(const std::string& identity, int option = LOG_PID, int facility = LOG_USER) :
        m_backend(identity, option, facility)
    {
        static_assert(std::is_enum<Level>::value, "level type must be enum");
    }

    void consume(Level level, const std::string& message) {
        priority_t priority = priority_traits<Level>::map(level);
        m_backend.write(priority, message);
    }

    Backend& backend() {
        return m_backend;
    }
};

} // namespace sink

template<typename Level>
struct factory_traits<sink::syslog_t<Level>> {
    typedef sink::syslog_t<Level> sink_type;
    typedef typename sink_type::config_type config_type;

    static void map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        ex["identity"].to(config.identity);
    }
};

} // namespace blackhole
