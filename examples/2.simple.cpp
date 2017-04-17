/// This example demonstrates the a bit extended the Blackhole logging library usage and its
/// features, like:
///  - Custom severity mapping.
///  - Logger construction using builder pattern.

#include <array>

#include <blackhole/extensions/writer.hpp>
#include <blackhole/formatter.hpp>
#include <blackhole/formatter/string.hpp>
#include <blackhole/handler.hpp>
#include <blackhole/handler/blocking.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/root.hpp>
#include <blackhole/sink.hpp>
#include <blackhole/sink/console.hpp>

/// As always specify severity enumeration.
enum severity {
    debug   = 0,
    info    = 1,
    warning = 2,
    error   = 3
};

/// Severity mapping function.
static auto sevmap(std::size_t severity, const std::string& spec, blackhole::writer_t& writer) -> void {
    static const std::array<const char*, 4> mapping = {{"D", "I", "W", "E"}};

    if (severity < mapping.size()) {
        writer.write(spec, mapping[severity]);
    } else {
        writer.write(spec, severity);
    }
}

auto main(int, char**) -> int {
    /// Here we are going to configure our string/console handler and to build the logger.
   auto log = blackhole::builder<blackhole::root_logger_t>()
       /// Add the blocking handler.
       .add(blackhole::builder<blackhole::handler::blocking_t>()
           /// Configure string formatter.
           ///
           /// Pattern syntax behaves like as usual substitution for placeholder. For example if
           /// the attribute named `severity` has value `2`, then pattern `{severity}` will invoke
           /// severity mapping function provided and the result will be `W`.
           .set(blackhole::builder<blackhole::formatter::string_t>("{severity}, [{timestamp}]: {message}")
               .mapping(&sevmap)
               .build())
           /// Configure console sink to write into stdout (also stderr can be configured).
           .add(blackhole::builder<blackhole::sink::console_t>()
               .build())
           /// And build the handler. Multiple handlers can be added to a single logger, but right
           /// now we confine ourselves with a single handler.
           .build())
       /// Build the logger.
       .build();

    log->log(severity::debug, "GET /static/image.png HTTP/1.1 404 347");
    log->log(severity::info, "nginx/1.6 configured");
    log->log(severity::warning, "client stopped connection before send body completed");
    log->log(severity::error, "file does not exist: /var/www/favicon.ico");

    return 0;
}
