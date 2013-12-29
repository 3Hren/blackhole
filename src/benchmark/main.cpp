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

namespace old {

class json_visitor_t : public boost::static_visitor<> {
    rapidjson::Document* root;
    const char* name;
public:
    json_visitor_t(rapidjson::Document* root) :
        root(root),
        name(nullptr)
    {}

    void bind_name(const char* name) {
        this->name = name;
    }

    template<typename T, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    void operator ()(T value) const {
        add_member(value);
    }

    void operator ()(std::time_t value) const {
        add_member(static_cast<int64_t>(value));
    }

    void operator ()(const std::string& value) const {
        add_member(value.c_str());
    }

private:
    template<typename T>
    void add_member(const T& value) const {
        root->AddMember(name, value, root->GetAllocator());
    }
};

namespace aux {

template<typename Visitor, typename T>
void apply_visitor_old(Visitor& visitor, const char* name, const T& value) {
    visitor.bind_name(name);
    boost::apply_visitor(visitor, value);
}

} // namespace aux

class json_t {
public:
    std::string format(const log::record_t& record) const {
        rapidjson::Document root;
        root.SetObject();

        json_visitor_t visitor(&root);
        for (auto it = record.attributes.begin(); it != record.attributes.end(); ++it) {
            const std::string& name = it->first;
            const log::attribute_t& attribute = it->second;
            aux::apply_visitor_old(visitor, name.c_str(), attribute.value);
        }

        rapidjson::GenericStringBuffer<rapidjson::UTF8<>> buffer;
        rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<>>> writer(buffer);

        root.Accept(writer);
        return std::string(buffer.GetString(), buffer.Size());
    }
};

} // namespace old

const int S = 0;
const int N = 500000;

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

old::json_t* fmt1 = 0;
formatter::json_t* fmt2 = 0;
formatter::json_t* fmt3 = 0;
formatter::json_t* fmt4 = 0;
formatter::json_t* fmt5 = 0;

int main(int argc, char** argv) {
    fmt1 = new old::json_t;

    formatter::json::config_t config2;
    config2.newline = true;
    fmt2 = new formatter::json_t(config2);

    formatter::json::config_t config3;
    config3.name_mapping["message"] = "@message";
    fmt3 = new formatter::json_t(config3);

    formatter::json::config_t config4;
    config4.fields_hierarchy["@source"] = { "fields" };
    config4.fields_hierarchy["@source_host"] = { "fields" };
    config4.fields_hierarchy["@uuid"] = { "fields" };
    fmt4 = new formatter::json_t(config4);

    formatter::json::config_t config5;
    config5.newline = true;
    config5.name_mapping["message"] = "@message";
    config5.fields_hierarchy["@source"] = { "fields" };
    config5.fields_hierarchy["@source_host"] = { "fields" };
    config5.fields_hierarchy["@uuid"] = { "fields" };
    fmt5 = new formatter::json_t(config5);

    celero::Run(argc, argv);
    return 0;
}

BASELINE(JsonFormatterBenchmark, Baseline, S, N) {
    celero::DoNotOptimizeAway(fmt1->format(record));
}

BENCHMARK(JsonFormatterBenchmark, NewLine, S, N) {
    celero::DoNotOptimizeAway(fmt2->format(record));
}

BENCHMARK(JsonFormatterBenchmark, Mapping, S, N) {
    celero::DoNotOptimizeAway(fmt3->format(record));
}

BENCHMARK(JsonFormatterBenchmark, BuildingTree, S, N) {
    celero::DoNotOptimizeAway(fmt4->format(record));
}

BENCHMARK(JsonFormatterBenchmark, Complex, S, N) {
    celero::DoNotOptimizeAway(fmt5->format(record));
}

#endif
