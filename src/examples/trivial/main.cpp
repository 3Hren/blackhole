#include <blackhole/blackhole.hpp>

//! This example demonstrates the simpliest blackhole logging library usage.

/*! All that we need - is to define severity enum with preferred log levels.
 *
 *  Logger object `verbose_logger_t` is provided by `repository_t` class, which
 *  requires severity enumeration as template parameter.
 */

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

int main(int, char**) {
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");
    return 0;
}
