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
BASELINE(CeleroBenchTest, Baseline, 100, N) {
    (boost::format("Le message: %s") % "le value").str();
}

using namespace blackhole;

log::

BENCHMARK(CeleroBenchTest, Benchmark, 100, N) {
}

#endif
