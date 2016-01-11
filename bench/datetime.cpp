#include <benchmark/benchmark.h>

#include <cstring>

#include <blackhole/extensions/format.hpp>

#include <blackhole/detail/datetime.hpp>

#include "mod.hpp"

namespace blackhole {
namespace benchmark {

static void datetime_strftime(::benchmark::State& state) {
    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        char buffer[64];
        strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_strftime_with_locale(::benchmark::State& state) {
    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        char buffer[64];
        strftime(buffer, 64, "%c", &tm);
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_wheel(::benchmark::State& state) {
    blackhole::detail::datetime::generator_t generator(
        blackhole::detail::datetime::make_generator("%Y-%m-%d %H:%M:%S")
    );

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        fmt::MemoryWriter wr;
        generator(wr, tm);
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_wheel_with_locale(::benchmark::State& state) {
    blackhole::detail::datetime::generator_t generator(
        blackhole::detail::datetime::make_generator("%c")
    );

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        fmt::MemoryWriter wr;
        generator(wr, tm);
    }

    state.SetItemsProcessed(state.iterations());
}

static void datetime_wheel_microseconds(::benchmark::State& state) {
    blackhole::detail::datetime::generator_t generator(
        blackhole::detail::datetime::make_generator("%Y-%m-%d %H:%M:%S.%f")
    );

    std::time_t time = std::time(0);
    std::tm tm;
    localtime_r(&time, &tm);

    while (state.KeepRunning()) {
        fmt::MemoryWriter wr;
        generator(wr, tm);
    }

    state.SetItemsProcessed(state.iterations());
}

namespace {

template<std::size_t length, char filler = '0'>
inline auto fill(int value, char* buffer) -> void {
    std::size_t digits = 0;
    do {
        buffer[length - 1 - digits] = (value % 10) + '0';
        value /= 10;
        ++digits;
    } while (value);

    std::memset(buffer, filler, length - digits);
}

template<std::size_t length, typename Writer, char filler = '0'>
inline void fill(Writer& wr, int value) {
    char buffer[std::numeric_limits<uint32_t>::digits10 + 2];
    fill<length, filler>(value, buffer);
    wr << fmt::StringRef(buffer, length);
}

}  // namespace

static void datetime_pure_real_world(::benchmark::State& state) {
    // The same as blackhole::detail::datetime::make_generator("%Y-%m-%d %H:%M:%S.%f").

    std::time_t time = std::time(0);
    std::tm tm;
    gmtime_r(&time, &tm);

    while (state.KeepRunning()) {
        fmt::MemoryWriter wr;
        fill<4>(wr, tm.tm_year + 1900);
        wr << "-";
        fill<2>(wr, tm.tm_mon + 1);
        wr << "-";
        fill<2>(wr, tm.tm_mday);
        wr << " ";
        fill<2>(wr, tm.tm_hour);
        wr << ":";
        fill<2>(wr, tm.tm_min);
        wr << ":";
        fill<2>(wr, tm.tm_sec);
        wr << ".";
        fill<6>(wr, 0);
    }

    state.SetItemsProcessed(state.iterations());
}

NBENCHMARK("datetime.manual[real]", datetime_pure_real_world);
NBENCHMARK("datetime.strftime", datetime_strftime);
NBENCHMARK("datetime.strftime[locale]", datetime_strftime_with_locale);
NBENCHMARK("datetime.blackhole", datetime_wheel);
NBENCHMARK("datetime.blackhole[locale]", datetime_wheel_with_locale);
NBENCHMARK("datetime.blackhole[with microseconds]", datetime_wheel_microseconds);

}  // namespace benchmark
}  // namespace blackhole
