#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

int main(int, char**) {
    verbose_logger_t<level> log = repository_t<level>::instance().trivial();

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");
    return 0;
}
