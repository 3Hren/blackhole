#include "global.hpp"

#include <blackhole/blackhole.hpp>
#include <blackhole/synchronized.hpp>

using namespace blackhole;

TEST(SynchronizedLogger, Class) {
    verbose_logger_t<level> log;
    synchronized<verbose_logger_t<level>> logger(std::move(log));
    UNUSED(logger);
}
