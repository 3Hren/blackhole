#include <ticktack/benchmark.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/severity.hpp>
#include <blackhole/keyword/timestamp.hpp>

namespace { enum level_t { info }; }

namespace {

void map_severity(blackhole::stickystream_t& stream, level_t level) {
    static const char* descriptions[] = { "INFO" };
    if (static_cast<std::size_t>(level) < sizeof(descriptions) / sizeof(*descriptions) && level >= 0) {
        stream << descriptions[level];
    } else {
        stream << level;
    }
}

int initialize(blackhole::formatter::string_t& formatter) {
    blackhole::mapping::value_t mapper;
    mapper.add<blackhole::keyword::tag::timestamp_t>("%Y-%m-%d %H:%M:%S.%f");
    mapper.add<level_t>("severity", &map_severity);
    formatter.set_mapper(mapper);
    return 0;
}

static const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

} // namespace

BENCHMARK(PureStringFormatter, Baseline) {
    static blackhole::formatter::string_t formatter(
        "[%(timestamp)s] [%(severity)s]: %(message)s"
    );
    static int initializer = initialize(formatter);
    ticktack::compiler::do_not_optimize(initializer);

    blackhole::record_t record;
    record.insert(blackhole::keyword::message() = MESSAGE_LONG);

    timeval tv;
    gettimeofday(&tv, nullptr);
    record.insert(blackhole::keyword::timestamp() = tv);
    record.insert(blackhole::keyword::severity<level_t>() = level_t::info);
    ticktack::compiler::do_not_optimize(formatter.format(record));
}

BENCHMARK(VariadicStringFormatter, Baseline) {
    static blackhole::formatter::string_t formatter(
        "[%(timestamp)s] [%(severity)s]: %(message)s %(...:(:))s"
    );
    static int initializer = initialize(formatter);
    ticktack::compiler::do_not_optimize(initializer);

    blackhole::attribute::set_t global;
    blackhole::attribute::set_t scoped;
    scoped.insert(blackhole::attribute::make("id", 42));
    scoped.insert(blackhole::attribute::make("source", "app/benchmark"));

    blackhole::attribute::set_t local;
    timeval tv;
    gettimeofday(&tv, nullptr);
    local.insert(blackhole::keyword::timestamp() = tv);
    local.insert(blackhole::keyword::severity<level_t>() = level_t::info);
    local.insert(blackhole::keyword::message() = MESSAGE_LONG);

    blackhole::attribute::set_view_t view(global, scoped, std::move(local));
    blackhole::record_t record(std::move(view));
    ticktack::compiler::do_not_optimize(formatter.format(record));
}
