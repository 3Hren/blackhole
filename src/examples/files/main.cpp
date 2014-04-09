#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/files.hpp>

//! This example demonstrates files sink usage with rotation support and other features.
//!  - how to register non-default formatter and sink in the repository.
//!  - how to configure files sink to support files rotation by size and datetime.
//!  - how to write in multiple files depending on current log event's attributes set.
//!  - how to attach additional attributes to the log event.

using namespace blackhole;

// As always define severity enumeration.
enum class level {
    debug,
    error
};

void init() {
    // This combination of formatter/sink is not default for Blackhole, thus it must be registered
    // in the repository. After registering new loggers can be created with configuration that uses
    // newly registered formatter's or sink's features.
    // You can look consider this action as some kind of plugin system.
    repository_t::instance().configure<
        sink::files_t<
            sink::files::boost_backend_t,
            sink::rotator_t<
                sink::files::boost_backend_t,
                sink::rotation::watcher::size_t
            >
        >,
        formatter::string_t
    >();

    // Formatter configuration is almost standard.
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    // But sink configuration opens more intresting features.
    sink_config_t sink("files");
    sink["path"] = "blackhole-%(host)s.log";
    sink["autoflush"] = true;
    sink["rotation"]["pattern"] = "%(filename)s.%N";
    sink["rotation"]["backups"] = std::uint16_t(10);
    sink["rotation"]["size"] = std::uint64_t(10 * 1024);

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    // See that 'second' invocation after macro? That's how additional attributes are attached to
    // the log event.
    for (int i = 0; i < 32; ++i) {
        BH_LOG(log, level::debug, "debug event")("host", "127.0.0.1");
        BH_LOG(log, level::error, "error event")("host", "localhost");
    }

    return 0;
}
