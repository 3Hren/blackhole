#include <ticktack/benchmark.hpp>

#include <blackhole/formatter/string.hpp>
#include <blackhole/logger.hpp>
#include <blackhole/macro.hpp>
#include <blackhole/sink/null.hpp>
#include <blackhole/utils/unique.hpp>

namespace { enum level_t { info }; }

namespace {

blackhole::verbose_logger_t<level_t> initialize() {
    auto formatter = blackhole::utils::make_unique<
        blackhole::formatter::string_t
    >("%(message)s");

    auto sink = blackhole::utils::make_unique<
        blackhole::sink::null_t
    >();

    auto frontend = blackhole::utils::make_unique<
        blackhole::frontend_t<
            blackhole::formatter::string_t,
            blackhole::sink::null_t
        >
    >(std::move(formatter), std::move(sink));

    blackhole::verbose_logger_t<level_t> log;
    log.add_frontend(std::move(frontend));
    return log;
}

static const char MESSAGE_LONG[] = "Something bad is going on but I can handle it";

} // namespace

BENCHMARK(LogStringToNull, Baseline) {
    static auto log = initialize();

    BH_LOG(log, level_t::info, MESSAGE_LONG)(
        "answer", 42,
        "string", "le string"
    );
}

// 1. create 3 maps. (global, thread + scoped, user)
// 2. merge them.
// 3. make lookup by 1-2 items.
// 4. return record.

#include <blackhole/record.hpp>

using namespace blackhole;

BENCHMARK_BASELINE(OpenRecord, OldStyle) {
    static log::attributes_t global = {
        std::make_pair("g10", log::attribute_t("ten")),
    };

    log::attributes_t thread = {
        std::make_pair("t10", log::attribute_t("ten")),
        std::make_pair("t20", log::attribute_t("twenty")),
    };

    log::attributes_t local = {
        std::make_pair("l10", log::attribute_t(10)),
        std::make_pair("l20", log::attribute_t("twenty")),
        std::make_pair("l30", log::attribute_t("thirty")),
        std::make_pair("l40", log::attribute_t("fourty")),
    };

    log::attributes_t attributes = merge({global, thread, local});
    attributes["timestamp"] = log::attribute_t(100500);
    attributes["pid"] = log::attribute_t(42);

    log::record_t record;

    auto attribute = attributes["g10"];
    ticktack::compiler::do_not_optimize(attribute);

    record.attributes = std::move(attributes);

    log::record_t other_record(std::move(record));
    auto a1 = other_record.attributes["g10"];
    auto a2 = other_record.attributes["t10"];
    auto a3 = other_record.attributes["l40"];

    ticktack::compiler::do_not_optimize(a1);
    ticktack::compiler::do_not_optimize(a2);
    ticktack::compiler::do_not_optimize(a3);
}

class attribute_view_t {
    log::attributes_t g;
    log::attributes_t t;
    log::attributes_t u;

    log::attributes_t o;
public:
    attribute_view_t(log::attributes_t&& g, log::attributes_t&& t, log::attributes_t&& u) :
        g(std::move(g)),
        t(std::move(t)),
        u(std::move(u))
    {}

    void insert(std::string k, log::attribute_t v) {
        o[std::move(k)] = std::move(v);
    }

    log::attribute_t operator[](const std::string& key) const noexcept {
        auto it = u.find(key);
        if (it != u.end()) {
            return it->second;
        }

        it = o.find(key);
        if (it != o.end()) {
            return it->second;
        }

        it = g.find(key);
        if (it != g.end()) {
            return it->second;
        }

        it = t.find(key);
        if (it != t.end()) {
            return it->second;
        }

        BOOST_ASSERT(false);
    }
};

struct record2_t {
    attribute_view_t attributes;
};

BENCHMARK_RELATIVE(OpenRecord, NewStyle) {
    static log::attributes_t global = {
        std::make_pair("g10", log::attribute_t("ten")),
    };

    log::attributes_t thread = {
        std::make_pair("t10", log::attribute_t("ten")),
        std::make_pair("t20", log::attribute_t("twenty")),
    };

    log::attributes_t local = {
        std::make_pair("l10", log::attribute_t(10)),
        std::make_pair("l20", log::attribute_t("twenty")),
        std::make_pair("l30", log::attribute_t("thirty")),
        std::make_pair("l40", log::attribute_t("fourty")),
    };

    attribute_view_t view(std::move(global), std::move(thread), std::move(local));
    view.insert("timestamp", log::attribute_t(100500));
    view.insert("pid", log::attribute_t(42));


    auto attribute = view["g10"];
    ticktack::compiler::do_not_optimize(attribute);

    record2_t record { std::move(view) };

    record2_t other_record(std::move(record));
    auto a1 = other_record.attributes["g10"];
    auto a2 = other_record.attributes["t10"];
    auto a3 = other_record.attributes["l40"];

    ticktack::compiler::do_not_optimize(a1);
    ticktack::compiler::do_not_optimize(a2);
    ticktack::compiler::do_not_optimize(a3);
}
