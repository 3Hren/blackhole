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

int main(int argc, char** argv) {
    auto f = std::make_unique<formatter::string_t>("[%(timestamp_id)s]: %(message)s");
    auto s = std::make_unique<sink::file_t<>>("test.log");
    auto frontend = std::make_unique<frontend_t<formatter::string_t, sink::file_t<>>>(std::move(f), std::move(s));
    log_.add_frontend(std::move(frontend));
    celero::Run(argc, argv); return 0;
}


BASELINE(CeleroBenchTest, Baseline, 10, N) {
    (boost::format("Le message: %s") % "le value").str();
}

BENCHMARK(CeleroBenchTest, Benchmark, 10, N) {
    auto record = log_.open_record();
    if (record.valid()) {
        record.attributes["message"] = utils::format("Le message: %s", "le value");
        log_.push(std::move(record));
    }
}
#else
#endif
