#include "blackhole/sink/file.hpp"

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/sink/file.hpp"

namespace blackhole {
namespace sink {

file_t::file_t(const std::string& path) :
    inner(new inner_t)
{}

file_t::~file_t() = default;

auto file_t::filter(const record_t& record) -> bool {
    return true;
}

auto file_t::execute(const record_t& record, const string_view& formatted) -> void {}

}  // namespace sink

auto factory<sink::file_t>::type() -> const char* {
    return "file";
}

}  // namespace blackhole
