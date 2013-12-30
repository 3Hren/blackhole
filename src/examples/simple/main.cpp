#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

namespace blackhole { namespace sink {

template<>
struct priority_traits<level> {
    static inline
    priority_t map(level lvl) {
        switch (lvl) {
        case level::debug:
            return priority_t::debug;
        case level::info:
            return priority_t::info;
        case level::warning:
            return priority_t::warning;
        case level::error:
            return priority_t::err;
        }

        return priority_t::debug;
    }
};

} } // namespace blackhole::sink

//! Initialization stage.
//! \brief Manually or from file - whatever. The main aim - is to get initialized `log_config_t` object.
void init() {
    formatter_config_t formatter = {
        "string",
        std::string("[%(timestamp)s] [%(severity)s]: %(message)s")
    };

    sink_config_t sink = {
        "files",
        std::map<std::string, std::string>{ { "path", "/dev/stdout" } }
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
