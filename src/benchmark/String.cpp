#include <ticktack/benchmark.hpp>

BENCHMARK(Vector, Simple) {
    std::vector<int> v;
    for (int i = 0; i < 1000; ++i) {
        v.push_back(i);
    }
}
