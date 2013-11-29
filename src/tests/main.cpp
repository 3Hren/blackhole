#include "Global.hpp"

#if 1
int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#else
#include "celero/Celero.h"

const int N = 10000;

CELERO_MAIN

BASELINE(CeleroBenchTest, Baseline, 0, N)
{
    char buf[128];
    sprintf(buf, "[%s]: %s", "le timestamp", "le message");
}

blackhole::log::record_t record {{
    { "message", "le message" },
    { "timestamp", "le timestamp" }
}};
std::string pattern("[%(timestamp)s]: %(message)s");
blackhole::formatter::string_t fmt(pattern);

BENCHMARK(CeleroBenchTest, Benchmark, 0, N) {
    celero::DoNotOptimizeAway(fmt.format(record));
}

#endif
