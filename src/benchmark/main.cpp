#include <memory>

#include "celero/Celero.h"

#include "../tests/Mocks.hpp"

using namespace blackhole;

//#define VS_BOOST
#ifdef VS_BOOST
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
    sink->set_formatter(expr::format("[%1%] [%2%]: %3%")
                        % expr::format_date_time(timestamp, "%Y-%m-%d %H:%M:%S")
                        % expr::attr<severity_level>("Severity")
                        % expr::smessage);

    logging::core::get()->add_sink(sink);
    logging::core::get()->set_filter(severity >= info);

    // Add attributes
    logging::add_common_attributes();
}

const int N = 10000;

boost::log::sources::severity_logger<severity_level> slg;
verbose_logger_t<severity_level> log_;

std::string map_timestamp(const std::time_t& time) {
    char mbstr[128];
    if(std::strftime(mbstr, 128, "%F %T", std::gmtime(&time))) {
        return std::string(mbstr);
    }
    return std::string("?");
}

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

#define BH_LOG(level, msg) \
    auto record = log_.open_record(level); \
    if (record.valid()) { \
        record.attributes["message"] = { msg }; \
        log_.push(std::move(record)); \
    }

void init_blackhole_log() {
    mapping::mapper_t mapper;
    mapper.add<std::time_t>("timestamp", &map_timestamp);
    mapper.add<severity_level>("severity", &map_severity);

    auto f = std::make_unique<formatter::string_t>("[%(timestamp)s] [%(severity)s]: %(message)s");
    f->set_mapper(std::move(mapper));
    auto s = std::make_unique<sink::file_t<>>("blackhole.log");
    auto frontend = std::make_unique<frontend_t<formatter::string_t, sink::file_t<>>>(std::move(f), std::move(s));
    log_.add_frontend(std::move(frontend));
}

int main(int argc, char** argv) {
    init_boost_log();
    init_blackhole_log();

    celero::Run(argc, argv);
    return 0;
}


BASELINE(CeleroBenchTest, BoostLog, 0, N) {
    BOOST_LOG_SEV(slg, warning) << "Something bad is going on but I can handle it";
}

BENCHMARK(CeleroBenchTest, BlackholeLog, 0, N) {
    BH_LOG(warning, "Something bad is going on but I can handle it");
}
#endif

#if 1
using namespace blackhole;

class json_t_old {
public:
    std::string format(const log::record_t& record) const {
        rapidjson::Document root;
        root.SetObject();

        formatter::json_visitor_t visitor(&root);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const std::string& name = it->first;
            const log::attribute_t& attribute = it->second;
            formatter::aux::apply_visitor(visitor, name.c_str(), attribute.value);
        }

        rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
        rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);

        root.Accept(writer);
        return std::string(buffer.GetString(), buffer.Size());
    }
};

const int N = 10000;

log::record_t record = {
    log::attributes_t{
        keyword::message() = "le message",
        keyword::timestamp() = 100500,
        attribute::make("@source", "udp://127.0.0.1"),
        attribute::make("@source_host", "dhcp-218-248-wifi.yandex"),
        attribute::make("@source_path", "service/storage"),
        attribute::make("@uuid", "550e8400-e29b-41d4-a716-446655440000")
    }
};

json_t_old fmt1;
formatter::json_t::config_type config;
formatter::json_t fmt2(config);

int main(int argc, char** argv) {
    config.mapping["message"] = "@message";
    celero::Run(argc, argv);
    return 0;
}

BASELINE(JsonFormatterBenchmark, Baseline, 0, N) {
    celero::DoNotOptimizeAway(fmt1.format(record));
}

BASELINE(JsonFormatterBenchmark, Mapping, 0, N) {
    celero::DoNotOptimizeAway(fmt2.format(record));
}

#endif
