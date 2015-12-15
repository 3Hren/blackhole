#include "blackhole/sink/file.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/sink/file.hpp"

namespace blackhole {
namespace sink {

file_t::file_t(const std::string& filename) :
    inner(new file::inner_t(filename, 0))
{}

file_t::file_t(std::unique_ptr<file::inner_t> inner) noexcept :
    inner(std::move(inner))
{}

file_t::file_t(file_t&& other) noexcept = default;

file_t::~file_t() = default;

auto file_t::operator=(file_t&& other) noexcept -> file_t& = default;

auto file_t::filter(const record_t&) -> bool {
    return true;
}

auto file_t::execute(const record_t& record, const string_view& formatted) -> void {
    const auto filename = inner->filename(record);

    std::lock_guard<std::mutex> lock(inner->mutex);
    inner->backend(filename).write(formatted);
}

}  // namespace sink

auto factory<sink::file_t>::type() -> const char* {
    return "file";
}

}  // namespace blackhole
