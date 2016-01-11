#pragma once

// Some private magic, but it's okay, since I manage the library version myself.
#define NBENCHMARK(name, n) \
    BENCHMARK_PRIVATE_DECLARE(n) =                               \
        (::benchmark::internal::RegisterBenchmarkInternal(       \
            new ::benchmark::internal::FunctionBenchmark(name, n)))
