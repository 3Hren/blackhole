#!/bin/sh

if [ "$SUITE" = "tests" ]; then
    if [ "$TRAVIS_OS_NAME" = "linux" ]; then
        cmake -DENABLE_TRACING_FRAMEWORK=ON -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=ON -DENABLE_TESTING_COVERAGE=ON .. && make
        make coverage
        coveralls-lcov coverage.info.cleaned
    else
        cmake -DENABLE_TRACING_FRAMEWORK=ON -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=ON .. && make
        ./src/tests/blackhole-tests
    fi
elif [ "$SUITE" = "examples" ]; then    
    cmake -DENABLE_TRACING_FRAMEWORK=ON -DENABLE_ELASTICSEARCH=ON -DENABLE_EXAMPLES=ON .. && make
elif [ "$SUITE" = "benchmarks" ]; then
    cmake -DENABLE_TRACING_FRAMEWORK=ON -DENABLE_ELASTICSEARCH=ON -DENABLE_BENCHMARKING=ON .. && make
    ./src/benchmark/blackhole-benchmark
else
    echo "Unknown suite variable - '$SUITE'. Terminating ..."
    exit 1
fi
