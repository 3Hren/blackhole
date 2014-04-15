#include "global.hpp"

#include <blackhole/blackhole.hpp>
#include <blackhole/synchronized.hpp>

using namespace blackhole;

TEST(SynchronizedLogger, Class) {
    synchronized<verbose_logger_t<level>> logger;
    UNUSED(logger);
}

TEST(SynchronizedLogger, ExplicitMoveConstructor) {
    synchronized<verbose_logger_t<level>> logger;
    synchronized<verbose_logger_t<level>> other(std::move(logger));
    UNUSED(other);
}

TEST(SynchronizedLogger, ImplicitMoveConstructor) {
    synchronized<verbose_logger_t<level>> logger;
    synchronized<verbose_logger_t<level>> other = std::move(logger);
    UNUSED(other);
}

TEST(SynchronizedLogger, ExplicitMoveConstructorForLogger) {
    verbose_logger_t<level> log;
    synchronized<verbose_logger_t<level>> logger(std::move(log));
    UNUSED(logger);
}

TEST(SynchronizedLogger, MoveAssignment) {
    verbose_logger_t<level> log;
    synchronized<verbose_logger_t<level>> logger(std::move(log));
    synchronized<verbose_logger_t<level>> other;
    other = std::move(logger);
}
