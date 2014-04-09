#include <blackhole/blackhole.hpp>

//! This example demonstrates the a bit extended blackhole logging library usage and its features.
//!  - detailed formatters and sinks configuration;
//!  - logger usage without macro.

using namespace blackhole;

// As always specify severity enumeration.
enum class level {
    debug
};

// Here we are going to configure our string/stream frontend and to register it.
void init() {
    // Configure string formatter.
    // Pattern syntax behaves like usual substitution for placeholder. For example if attribute
    // named `severity` has value `2`, then pattern [%(severity)s] will produce: [2].
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    // Configure stream sink to write into stdout (also stderr can be configured).
    sink_config_t sink("stream");
    sink["output"] = "stdout";

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

// Here it's an example how to create and handle log events without using any macros at all.
template<typename Log>
void handle(Log& log, level lvl, const char* message) {
    // Tries to create log record. Returns invalid record object if created log record couldn't
    // pass filtering stage.
    // For our case it will be created anyway, because we don't have any filters registered now.
    log::record_t record = log.open_record(lvl);
    if (record.valid()) {
        // Manually insert message attribute into log record attributes set using keyword API.
        // Formatter will break up if some attributes it needed don't exist where formatting
        // occures.
        record.attributes.insert({
            keyword::message() = message
        });
        log.push(std::move(record));
    }
}

int main(int, char**) {
    init();
    verbose_logger_t<level> log = repository_t::instance().root<level>();

    BH_LOG(log, level::debug, "log message using macro API");
    handle(log, level::debug, "log message using log object directly");

    return 0;
}
