#include "Global.hpp"

#include <memory>

#if 1
int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#else
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

#endif
