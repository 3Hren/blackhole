#include <ticktack/benchmark.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/keyword/message.hpp>
#include <blackhole/keyword/severity.hpp>
#include <blackhole/keyword/timestamp.hpp>

namespace { enum level_t { info }; }

namespace {

void map_severity(blackhole::aux::attachable_ostringstream& stream, level_t level) {
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

    blackhole::log::record_t record;
    record.attributes.insert(
        blackhole::keyword::message() = MESSAGE_LONG
    );

    timeval tv;
    gettimeofday(&tv, nullptr);
    record.attributes.insert(blackhole::keyword::timestamp() = tv);
    record.attributes.insert(blackhole::keyword::severity<level_t>() = level_t::info);
    ticktack::compiler::do_not_optimize(formatter.format(record));
}
