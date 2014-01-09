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

int main(int, char**) {
    verbose_logger_t<level> log = repository_t<level>::instance().trivial();

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");
    return 0;
}
