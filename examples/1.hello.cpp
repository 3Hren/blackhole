//! This example demonstrates the a very basic the Blackhole logging library usage and its features,
//! like:
//!  - Logger construction using builder pattern.
//!  - Logging using logger methods.

#include <blackhole/builder.hpp>
#include <blackhole/handler/dev.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/root.hpp>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/extensions/facade.hpp>
#include <blackhole/extensions/writer.hpp>

auto main() -> int {
    // Here we are going to configure our development handler and to build the logger.
    auto lg = blackhole::experimental::partial_builder<blackhole::root_logger_t>()
        // Add the development handler.
        .handler<blackhole::handler::dev_t>()
            .build()
        // Build the logger.
        .build();
    auto log = blackhole::logger_facade<blackhole::root_logger_t>(lg);

    log.log(0, "add {} quads for {}, weight: {:.2f}%, {}/{}", 64, "nginx", 34.375, 22, 64,
        blackhole::attribute_list {
            {"source",   "locator"},
            {"group",    "nginx_group1466158150"},
            {"service",  "locator"},
            {"endpoint", "[::]:38502"}
        }
    );

    log.log(1, "stopping engine", {
        {"engine", 4},
        {"source", "core/asio"},
    });

    log.log(2, "core has been terminated");
    log.log(3, "client stopped connection before send body completed", {
        {"uuid",     "a7c345fa-2034-439e-b941-44321419725e"},
        {"endpoint", "[::]:59010"}
    });
    log.log(4, "file does not exist: {}", "/var/www/favicon.ico");

    return 0;
}
