/// This example demonstrates how to initialize Blackhole from configuration file using JSON
/// builder.
/// In this case the entire logging pipeline is initializaed from file including severity mapping.

#include <fstream>
#include <iostream>

#include <blackhole/config/json.hpp>
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
    auto log = blackhole::registry_t::configured()
        /// Specify the concrete builder type we want to use. It may be JSON, XML, YAML or whatever
        /// else.
        .builder<blackhole::config::json_t>(std::ifstream(argv[1]))
            /// Build the logger named "root".
            .build("root");

    log.log(severity::debug, "GET /static/image.png HTTP/1.1 404 347");
    log.log(severity::info, "nginx/1.6 configured");
    log.log(severity::warning, "client stopped connection before send body completed");
    log.log(severity::error, "file does not exist: /var/www/favicon.ico");

    return 0;
}
