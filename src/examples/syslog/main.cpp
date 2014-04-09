#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/syslog.hpp>

//! This example demonstrates how to configure syslog sink and its features.
//!  - mapping from user-defined severity to the syslog's one.

enum class level {
    debug,
    warning,
    error
};

// To be able to properly map user-defined severity enumeration to the syslog's one
// we should implement special mapping trait that is called by library each time when
// mapping is required.
namespace blackhole {

namespace sink {

template<>
struct priority_traits<level> {
    static priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::warning:
            return priority_t::warning;
        case level::error:
            return priority_t::err;
        default:
            return priority_t::debug;
        }

        return priority_t::debug;
    }
};

} // namespace sink

} // namespace blackhole

using namespace blackhole;

// Here we are going to configure our string/syslog frontend and to register it.
void init() {
    // As always register necessary formatter and sink. Note that syslog sink requires
    // user-defined severity enumeration symbol as template parameter.
    // This information is needed for severity level mapping.
    repository_t::instance().configure<sink::syslog_t<level>, formatter::string_t>();

    // Formatter is configured as usual, except we don't need anything than message.
    formatter_config_t formatter("string");
    formatter["pattern"] = "%(message)s";

    // Syslog sink in its current implementation also hasn't large amout of options.
    sink_config_t sink("syslog");
    sink["identity"] = "test-application";

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug, "debug message");
    BH_LOG(log, level::warning, "warning message");
    BH_LOG(log, level::error, "error message");

    return 0;
}
