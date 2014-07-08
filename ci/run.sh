#!/bin/sh

if [ "$SUITE" = "tests" ]; then
    echo "cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=ON -DENABLE_EXAMPLES=OFF .. && make"
    cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=ON -DENABLE_EXAMPLES=OFF .. && make
    ./src/tests/blackhole-tests
elif [ "$SUITE" = "examples" ]; then
    echo "cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=OFF -DENABLE_EXAMPLES=ON .. && make"
    cmake -DENABLE_ELASTICSEARCH=ON -DENABLE_TESTING=OFF -DENABLE_EXAMPLES=ON .. && make
else
    echo "Unknown suite variable - '$SUITE'. Terminating ..."
    exit 1
fi
