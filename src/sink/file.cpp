#include "blackhole/sink/file.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/sink/file.hpp"

namespace blackhole {
namespace sink {

class file_t::properties_t {
public:
    std::string filename;
    std::size_t interval;
};

file_t::file_t(const std::string& filename) :
    inner(new file::inner_t(filename, 0))
{}

file_t::file_t(std::unique_ptr<file::inner_t> inner) noexcept :
    inner(std::move(inner))
{}

file_t::file_t(std::unique_ptr<file_t::properties_t> properties) :
    inner(new file::inner_t(properties->filename, properties->interval))
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

file_t::builder_t::builder_t(const std::string& filename) :
    properties(new properties_t{filename, 0})
{}

file_t::builder_t::builder_t(builder_t&& other) noexcept = default;

file_t::builder_t::~builder_t() = default;

auto file_t::builder_t::operator=(builder_t&& other) noexcept -> builder_t& = default;

auto file_t::builder_t::interval(std::size_t count) -> builder_t& {
    properties->interval = count;
    return *this;
}

auto file_t::builder_t::build() -> file_t {
    return file_t(std::move(properties));
}

}  // namespace sink

auto factory<sink::file_t>::type() -> const char* {
    return "file";
}

}  // namespace blackhole
