/// This example demonstrates the a bit extended the Blackhole logging library usage and its
/// features, like:
///  - Logger construction using builder pattern.

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

auto main(int, char**) -> int {
    std::vector<std::unique_ptr<blackhole::handler_t>> handlers;
    handlers.push_back(blackhole::builder<blackhole::handler::blocking_t>()
        .set(blackhole::builder<blackhole::formatter::string_t>("{severity}, [{timestamp}]: {message}")
            .build())
        .add(blackhole::builder<blackhole::sink::console_t>()
            .stdout()
            .build())
        .build());

    auto log = blackhole::root_logger_t(std::move(handlers));

    log.log(severity::debug, "GET /static/image.png HTTP/1.1 404 347");
    log.log(severity::info, "nginx/1.6 configured");
    log.log(severity::warning, "client stopped connection before send body completed");
    log.log(severity::error, "file does not exist: /var/www/favicon.ico");

    return 0;
}
