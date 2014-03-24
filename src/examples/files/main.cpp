#include <blackhole/formatter/string.hpp>
#include <blackhole/frontend.hpp>
#include <blackhole/frontend/files.hpp>
#include <blackhole/log.hpp>
#include <blackhole/repository.hpp>
#include <blackhole/sink/files.hpp>

using namespace blackhole;

enum class level {
    debug,
    info,
    warning,
    error
};

void init() {
    repository_t<level>::instance().configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();

    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "A-%(severity)s.log";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.log.%N";
    sink["rotation"]["backups"] = std::uint16_t(10);
    sink["rotation"]["size"] = std::uint64_t(10 * 1024);

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t<level>::instance().add_config(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t<level>::instance().root();

    for (int i = 0; i < 32; ++i) {
    BH_LOG(log, level::debug,   "[%d] %s - done", 0, "debug");
    BH_LOG(log, level::info,    "[%d] %s - done", 1, "info");
    BH_LOG(log, level::warning, "[%d] %s - done", 2, "warning");
    BH_LOG(log, level::error,   "[%d] %s - done", 3, "error");
    }

    return 0;
}
