#include <ticktack/benchmark.hpp>

#include <blackhole/detail/datetime.hpp>

BENCHMARK_BASELINE(DatetimeGenerator, Strftime) {
    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);
    char buffer[64];
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    ticktack::compiler::do_not_optimize(buffer);
}

BENCHMARK_RELATIVE(DatetimeGenerator, StrftimeUsingLocale) {
    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);
    char buffer[64];
    strftime(buffer, 64, "%c", &tm);
    ticktack::compiler::do_not_optimize(buffer);
}

BENCHMARK_RELATIVE(DatetimeGenerator, Generator) {
    static std::string buffer;
    static blackhole::aux::datetime::generator_t generator(
        blackhole::aux::datetime::generator_factory_t::make("%Y-%m-%d %H:%M:%S")
    );
    static blackhole::aux::attachable_ostringstream stream(buffer);

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);
    generator(stream, tm);
    ticktack::compiler::do_not_optimize(buffer);
    buffer.clear();
}

BENCHMARK_RELATIVE(DatetimeGenerator, GeneratorUsingLocale) {
    static std::string buffer;
    static blackhole::aux::datetime::generator_t generator(
        blackhole::aux::datetime::generator_factory_t::make("%c")
    );
    static blackhole::aux::attachable_ostringstream stream(buffer);

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);
    generator(stream, tm);
    ticktack::compiler::do_not_optimize(buffer);
    buffer.clear();
}
