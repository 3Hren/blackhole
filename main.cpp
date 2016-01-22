#include <blackhole/extensions/format.hpp>
#include <blackhole/extensions/metaformat2.hpp>

using namespace blackhole;
using namespace blackhole::experimental;
using namespace blackhole::experimental::detail;

int main() {
    constexpr auto tuple = std::make_tuple(
        literal_t("remote "),
        placeholder_t("{}"),
        literal_t(" {"),
        literal_t("id}"),
        literal_t(" mismatch: '"),
        placeholder_t("{}"),
        literal_t("' vs. '"),
        placeholder_t("{}"),
        literal_t("'")
    );

    fmt::MemoryWriter wr;

#ifdef PIDOR
    wr << fmt::StringRef(std::get<0>(tuple).get().data(), std::get<0>(tuple).get().size());
    wr << "client";
    wr << fmt::StringRef(std::get<2>(tuple).get().data(), std::get<2>(tuple).get().size());
    wr << fmt::StringRef(std::get<3>(tuple).get().data(), std::get<3>(tuple).get().size());
    wr << fmt::StringRef(std::get<4>(tuple).get().data(), std::get<4>(tuple).get().size());
    wr << 1;
    wr << fmt::StringRef(std::get<6>(tuple).get().data(), std::get<6>(tuple).get().size());
    wr << 42;
    wr << fmt::StringRef(std::get<8>(tuple).get().data(), std::get<8>(tuple).get().size());
#endif

#ifdef HUI
    experimental::detail::tokenizer2::format(tuple, wr, "client", 1, 42);
#endif

    return wr.size();
}
