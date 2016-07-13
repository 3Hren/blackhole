/// This example demonstrates how to initialize Blackhole from configuration file using JSON
/// builder.
/// In this case the entire logging pipeline is initialized from file including severity mapping.
/// The logging facade is used to allow runtime formatting and attributes provisioning.

#include <fstream>
#include <iostream>

#include <blackhole/attribute.hpp>
#include <blackhole/attributes.hpp>
#include <blackhole/config/json.hpp>
#include <blackhole/extensions/facade.hpp>
#include <blackhole/extensions/writer.hpp>
#include <blackhole/registry.hpp>
#include <blackhole/root.hpp>

using namespace blackhole;

/// As always specify severity enumeration.
enum severity {
    debug   = 0,
    info    = 1,
    warning = 2,
    error   = 3
};

auto main(int argc, char** argv) -> int {
    if (argc != 2) {
        std::cerr << "Usage: 3.config PATH" << std::endl;
        return 1;
    }

    /// Here we are going to build the logger using registry. The registry's responsibility is to
    /// track registered handlers, formatter and sinks, but for now we're not going to register
    /// anything else, since there are predefined types.
    auto inner = blackhole::registry::configured()
        /// Specify the concrete builder type we want to use. It may be JSON, XML, YAML or whatever
        /// else.
        ->builder<blackhole::config::json_t>(std::ifstream(argv[1]))
            /// Build the logger named "root".
            .build("root");

    /// Wrap the logger with facade to obtain an ability to format messages and provide attributes.
    auto log = blackhole::logger_facade<blackhole::root_logger_t>(inner);

    log.log(severity::debug, "{} {} HTTP/1.1 {} {}", "GET", "/static/image.png", 404, 347);
    log.log(severity::info, "nginx/1.6 configured", {
        {"elapsed", 32.5}
    });
    log.log(severity::warning, "client stopped connection before send body completed");
    log.log(severity::error, "file does not exist: {}", "/var/www/favicon.ico", blackhole::attribute_list{
        {"Cache", true},
        {"Cache-Duration", 10},
        {"User-Agent", "Mozilla Firefox"}
    });

    return 0;
}
