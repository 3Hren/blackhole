#include <fstream>
#include <memory>

#include "celero/Celero.h"

#include <blackhole/blackhole.hpp>
#include <blackhole/frontend/files.hpp>
#include <blackhole/synchronized.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

using namespace blackhole;

//! Severity level.
enum level {
    debug,
    info,
    warning,
    error,
    critical
};

static const char* DESCRIPTION[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
};

//! Severity mapping functions.
//!     - for boost ...
std::ostream& operator<<(std::ostream& stream, level lvl) {
    if (static_cast< std::size_t >(lvl) < sizeof(DESCRIPTION) / sizeof(*DESCRIPTION)) {
        stream << DESCRIPTION[lvl];
    } else {
        stream << static_cast<int>(lvl);
    }

    return stream;
}

//!     - and for blackhole ...
void map_severity(blackhole::aux::attachable_ostringstream& stream, level lvl) {
    if (static_cast< std::size_t >(lvl) < sizeof(DESCRIPTION) / sizeof(*DESCRIPTION)) {
        stream << DESCRIPTION[lvl];
    } else {
        stream << static_cast<int>(lvl);
    }
}

//! Register severity keyword for boost logger.
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", level)

//! Initialialize boost logger.
void init_boost_log() {
    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink_t;

    // Create and configure files sink.
    boost::shared_ptr<text_sink_t> sink(boost::make_shared<text_sink_t>());
    sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>("boost.log"));
    sink->locked_backend()->auto_flush();
    sink->set_formatter(boost::log::expressions::format("[%1%]: %2%")
                        % boost::log::expressions::attr<level>("Severity")
                        % boost::log::expressions::smessage);

    boost::log::core::get()->add_sink(sink);
    boost::log::core::get()->set_filter(severity >= info);

    // Add common attributes.
    boost::log::add_common_attributes();
}

//! Initialialize blackhole logger.
void init_blackhole_log() {
    repository_t::instance().configure<sink::files_t<>, formatter::string_t>();

    mapping::value_t mapper;
    mapper.add<level>("severity", &map_severity);

    formatter_config_t formatter("string", mapper);
    formatter["pattern"] = "[%(severity)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "blackhole.log";
    sink["autoflush"] = true;

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t::instance().add_config(config);
}

//! Create logger objects.
boost::log::sources::severity_logger<level> boost_log;
verbose_logger_t<level> blackhole_log;

int main(int argc, char** argv) {
    init_boost_log();
    init_blackhole_log();
    auto log = repository_t::instance().root<level>();
    blackhole_log = std::move(log);

    celero::Run(argc, argv);
    return 0;
}

static const int SAMPLES = 30;
static const int CALLS = 50000;

BASELINE(Benchmark, BoostLog, SAMPLES, CALLS) {
    BOOST_LOG_SEV(boost_log, warning) << "Something bad is going on but I can handle it";
}

BENCHMARK(Benchmark, BlackholeLog, SAMPLES, CALLS) {
    BH_LOG(blackhole_log, warning, "Something bad is going on but I can handle it");
}
