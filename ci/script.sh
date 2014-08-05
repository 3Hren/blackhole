#!/bin/sh

if [ "$SUITE" = "tests" ]; then
    echo "cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=ON -DENABLE_EXAMPLES=OFF .. && make"
    cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=ON .. && make
    ./src/tests/blackhole-tests
elif [ "$SUITE" = "examples" ]; then    
    cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_EXAMPLES=ON .. && make
elif [ "$SUITE" = "benchmarks" ]; then
    cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_BENCHMARKING=ON .. && make
    ./src/benchmark/blackhole-benchmark
else
    echo "Unknown suite variable - '$SUITE'. Terminating ..."
    exit 1
fi
