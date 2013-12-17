#include <memory>

#include "celero/Celero.h"

#include "../tests/Mocks.hpp"

using namespace blackhole;

#define VS_BOOST
#ifdef VS_BOOST

const int N = 100000;

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
