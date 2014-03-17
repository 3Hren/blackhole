#include <memory>

#include "celero/Celero.h"

#include <blackhole/logger.hpp>
#include <blackhole/log.hpp>
#include <blackhole/sink/files.hpp>
#include <blackhole/repository.hpp>

using namespace blackhole;

#include <boost/log/utility/setup.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/expressions/formatters.hpp>

#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

enum severity_level {
    debug,
    info,
    warning,
    error,
    critical
};

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level);
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime);

std::ostream& operator<< (std::ostream& strm, severity_level level) {
    static const char* strings[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast<int>(level);

    return strm;
}

void init_boost_log() {
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
    sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>("boost.log"));
    sink->locked_backend()->auto_flush();
    sink->set_formatter(expr::format("[%1%]: %2%")
                        % expr::attr<severity_level>("Severity")
                        % expr::smessage);

    logging::core::get()->add_sink(sink);
    logging::core::get()->set_filter(severity >= info);

    // Add attributes
    logging::add_common_attributes();
}

const int N = 50000;

boost::log::sources::severity_logger<severity_level> slg;
verbose_logger_t<severity_level> *log_;

std::string map_severity(const severity_level& level) {
    static const char* strings[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        return strings[level];
    return std::to_string(static_cast<int>(level));
}

void init_blackhole_log() {
    repository_t<severity_level>::instance().configure<sink::files_t<>, formatter::string_t>();

    mapping::value_t mapper;
    mapper.add<severity_level>("severity", &map_severity);

    formatter_config_t formatter("string", mapper);
    formatter["pattern"] = "[%(severity)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "blackhole.log";
    sink["autoflush"] = true;

    frontend_config_t frontend = { formatter, sink };
    log_config_t config{ "root", { frontend } };

    repository_t<severity_level>::instance().add_config(config);
}

int main(int argc, char** argv) {
    init_boost_log();
    init_blackhole_log();
    auto log = repository_t<severity_level>::instance().root();
    log_ = &log;

    celero::Run(argc, argv);
    return 0;
}


BASELINE(CeleroBenchTest, BoostLog, 0, N) {
    BOOST_LOG_SEV(slg, warning) << "Something bad is going on but I can handle it";
}

BENCHMARK(CeleroBenchTest, BlackholeLog, 0, N) {
    BH_LOG((*log_), warning, "Something bad is going on but I can handle it");
}
