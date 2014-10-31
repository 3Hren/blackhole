#include <epicmeter/benchmark.hpp>

#include <blackhole/blackhole.hpp>

using namespace blackhole;

typedef std::vector<std::pair<std::string, attribute::value_t>> set_t;

static const int THRESHOLD = 0;

inline bool filter_new(const set_t& view) {
    auto it = std::find_if(view.begin(), view.end(), [](const set_t::value_type& p) {return p.first == "severity"; });
    return it != view.end() && (boost::get<int>(it->second) > THRESHOLD);
}

inline bool filter_old(const attribute::set_t& view) {
    auto it = view.find("severity");
    return it != view.end() && (boost::get<int>(it->second.value) > THRESHOLD);
}

static set_t init_wrapped() {
    set_t wrapped;
    wrapped.reserve(8);
    wrapped.emplace_back("w1", 1);
    wrapped.emplace_back("w2", 2);
    return wrapped;
}

static set_t init_scoped() {
    set_t scoped;
    scoped.reserve(8);
    scoped.emplace_back("s1", 10);
    scoped.emplace_back("s2", 20);
    return scoped;
}

static const set_t wrapped = init_wrapped();
static const set_t scoped  = init_scoped();

BENCHMARK(_, 1) {
    set_t internal, external;
    internal.reserve(4);
    external.reserve(16);

    internal.emplace_back("severity", 1);
    if ((::filter_new(internal))) {
        internal.emplace_back("message", "message is very large");
        internal.emplace_back("timestamp", 100500);

        auto inserter = std::back_inserter(external);
        std::copy(wrapped.begin(), wrapped.end(), inserter);
        std::copy(scoped.begin(), scoped.end(), inserter);

        auto it = std::find_if(internal.begin(), internal.end(), [](const set_t::value_type& p) {return p.first == "timestamp"; });
        epicmeter::compiler::do_not_optimize(it);

             it = std::find_if(internal.begin(), internal.end(), [](const set_t::value_type& p) {return p.first == "severity"; });
        epicmeter::compiler::do_not_optimize(it);

             it = std::find_if(internal.begin(), internal.end(), [](const set_t::value_type& p) {return p.first == "message"; });
        epicmeter::compiler::do_not_optimize(it);
    }
}

BENCHMARK(_, 2) {
    attribute::set_t internal, external;
    internal.reserve(4);
    external.reserve(16);

    internal.emplace(std::string("severity"), attribute::value_t(1));

    if ((::filter_old(internal))) {
        internal.emplace(std::string("message"), attribute::value_t("message is very large"));
        internal.emplace(std::string("timestamp"), attribute::value_t(100500));

        for (auto& i : wrapped) {
            external.insert(std::make_pair(i.first, i.second));
        }

        for (auto& i : scoped) {
            external.insert(std::make_pair(i.first, i.second));
        }

        auto it = internal.find("timestamp");
        epicmeter::compiler::do_not_optimize(it);

             it = internal.find("severity");
        epicmeter::compiler::do_not_optimize(it);

             it = internal.find("message");
        epicmeter::compiler::do_not_optimize(it);
    }
}
