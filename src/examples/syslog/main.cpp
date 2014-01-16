#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>

#include <blackhole/frontend/syslog.hpp>
#include <blackhole/sink/syslog.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

//! Priority mapping traits.
namespace blackhole {

namespace sink {

template<>
struct priority_traits<level> {
    static priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::info:
            return priority_t::info;
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

//! Initialization stage.
void init() {
    repository_t<level>::instance().configure<sink::syslog_t<level>, formatter::string_t>();

    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s] [%(severity)s]: %(message)s")
    };

    sink_config_t sink = {
        "syslog",
        std::string("test-application")
    };

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t<level>::instance().init(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t<level>::instance().root();

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");

    return 0;
}
