#pragma once

#include "syslog.h"

#include <string>

#include "blackhole/error.hpp"
#include "blackhole/repository/factory/traits.hpp"
#include "blackhole/sink/thread.hpp"

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
    const std::string identity;

public:
    native_t(std::string identity, int option = LOG_PID, int facility = LOG_USER) :
        identity(std::move(identity))
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
        if (identity.empty()) {
            throw error_t("no syslog identity has been specified");
        }

        ::openlog(identity.c_str(), option, facility);
    }
};

} // namespace backend

namespace syslog {

struct config_t {
    std::string identity;
    int option;
    int facility;

    config_t() :
        identity("blackhole"),
        option(LOG_PID),
        facility(LOG_USER)
    {}

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
public:
    typedef Level level_type;
    typedef Backend backend_type;
    typedef syslog::config_t config_type;

private:
    backend_type backend_;

public:
    syslog_t(const config_type& config) :
        backend_(config.identity, config.option, config.facility)
    {
        static_assert(std::is_enum<level_type>::value, "level type must be enum");
    }

    syslog_t(const std::string& identity, int option = LOG_PID, int facility = LOG_USER) :
        backend_(identity, option, facility)
    {
        static_assert(std::is_enum<level_type>::value, "level type must be enum");
    }

    static const char* name() {
        return "syslog";
    }

    void consume(level_type level, const std::string& message) {
        priority_t priority = priority_traits<level_type>::map(level);
        backend_.write(priority, message);
    }

#ifdef BLACKHOLE_TESTING
    backend_type& backend() {
        return backend_;
    }
#endif
};

template<typename Level>
struct thread_safety<syslog_t<Level>> :
    public std::integral_constant<
        thread::safety_t,
        thread::safety_t::safe
    >::type
{};

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
