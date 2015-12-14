#include "blackhole/sink/file.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/sink/file.hpp"

namespace blackhole {
namespace sink {

file_t::file_t(const std::string& filename) :
    inner(new inner_t(filename, 0))
{}

file_t::file_t(std::unique_ptr<inner_t> inner) noexcept :
    inner(std::move(inner))
{}

file_t::~file_t() = default;

auto file_t::filter(const record_t& record) -> bool {
    return true;
}

auto file_t::execute(const record_t& record, const string_view& formatted) -> void {
    // TODO: Obtain the proper filename: record, inner -> filename.
    const auto filename = inner->filename(record);

    // TODO: Obtain the proper I/O stream: filename -> ofstream.
    auto& backend = inner->backend(filename);

    // TODO: Write to the stream.
    backend.write(formatted);
}

}  // namespace sink

auto factory<sink::file_t>::type() -> const char* {
    return "file";
}

}  // namespace blackhole
