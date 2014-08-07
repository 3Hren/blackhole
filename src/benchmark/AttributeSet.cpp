#include <ticktack/benchmark.hpp>

#include <blackhole/attribute.hpp>

BENCHMARK_BASELINE(AttributeSetConstructor, Default) {
    std::unordered_map<std::string, blackhole::log::attribute_value_t> c;
    ticktack::compiler::do_not_optimize(c);
}
