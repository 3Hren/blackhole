#include "Global.hpp"

#include <memory>

/*! @brief Log library specification.
 *
 *  === TYPICAL USAGE VIA MACRO ===
 *
 *  LOG(log, debug, "some information: %s at [%d]", "nothing", 100500, {
 *      keyword::id(42),
 *      keyword::address("127.0.0.1")
 *  });
 *
 *  === TYPYCAL USAGE VIA LOGGER OBJECT ===
 *
 *  log.debug("some information: %s at [%d]", "nothing", 100500, {
 *      keyword::id(42),
 *      keyword::address("127.0.0.1")
 *  });
 *
 *  === YOU CAN CREATE OWN ATTRIBUTES DYNAMICALLY ===
 *
 *  attribute::make("name", "value");
 *
 *
 *  === FORMATTERS ===
 *
 * 1. string: pattern="[%(timestamp)s] [%(level)s]: %(message)s : [%(...L)s]"
 *
 *  === SINKS ===
 *
 * socket: host, port
 *   udp.
 *   tcp:
 *     sync.
 *     async:
 *       queue_strategy.
 *
 *  === USAGE FROM PYTHON ===
 *
 *  log.debug("message from '%s'", "Hell", { "owner": "Satan", "email": "satan@mail.hell" })
 *
 *  === CONFIGURING FROM FILE ===
 *
 * verbosity: info
 * sink: file
 *   path: [/]
 *   format: string
 *     pattern: [%(...)s] ...
 *
 * sink: tcp/udp
 *   address: 0.0.0.0
 *   port: 5000
 *   format: msgpack/json
 *
 * sink: syslog
 *   identity: blah
 *   format: string
 *     pattern: [%(...)s] ...
 *
 */

#if 1
int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#elif 0
#include "celero/Celero.h"

using namespace blackhole;

const int N = 100000;

//CELERO_MAIN

logger_base_t log_;

std::string map_timestamp(const std::time_t& time) {
    char mbstr[128];
    if(std::strftime(mbstr, 128, "%F %T", std::gmtime(&time))) {
        return std::string(mbstr);
    }
    return std::string("?");
}

int main(int argc, char** argv) {
    mapping::mapper_t mapper;
    mapper.add<std::time_t>("timestamp", &map_timestamp);

    auto f = std::make_unique<formatter::string_t>("[%(timestamp)s]: %(message)s");
    f->set_mapper(std::move(mapper));
    auto s = std::make_unique<sink::file_t<>>("test.log");
    auto frontend = std::make_unique<frontend_t<formatter::string_t, sink::file_t<>>>(std::move(f), std::move(s));
    log_.add_frontend(std::move(frontend));
    celero::Run(argc, argv);
    return 0;
}


BASELINE(CeleroBenchTest, Baseline, 0, N) {
    std::time_t time = std::time_t(nullptr);
    celero::DoNotOptimizeAway(map_timestamp(time));
    (boost::format("Le message: %s") % "le value").str();
}

BENCHMARK(CeleroBenchTest, Benchmark, 0, N) {
    auto record = log_.open_record();
    if (record.valid()) {
        record.attributes["message"] = { utils::format("Le message: %s", "le value") };
        log_.push(std::move(record));
    }
}
#else
#endif
