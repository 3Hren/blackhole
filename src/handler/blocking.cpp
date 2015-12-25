#include "blackhole/handler/blocking.hpp"

#include "blackhole/extensions/writer.hpp"
#include "blackhole/formatter.hpp"
#include "blackhole/sink.hpp"

namespace blackhole {
namespace handler {

auto
blocking_t::execute(const record_t& record) -> void {
    writer_t writer;

    formatter->format(record, writer);

    for (const auto& sink : sinks) {
        // TODO: Check for filter.
        sink->execute(record, writer.result());
    }
}

auto
blocking_t::set(std::unique_ptr<formatter_t> formatter) -> void {
    this->formatter = std::move(formatter);
}

auto
blocking_t::add(std::unique_ptr<sink_t> sink) -> void {
    sinks.emplace_back(std::move(sink));
}

}  // namespace handler

auto
factory<handler::blocking_t>::type() -> const char* {
    return "blocking";
}

auto
factory<handler::blocking_t>::from(const config::node_t&) -> handler::blocking_t {
    return {};
}

}  // namespace blackhole
