#include "blackhole/sink/file.hpp"

#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/sink/file.hpp"
#include "blackhole/detail/util/deleter.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

file_t::file_t(const std::string& filename) :
    inner(new file::inner_t(filename, 0))
{}

file_t::file_t(const file_properties_t& properties) :
    inner(new file::inner_t(properties.filename, properties.interval))
{}

auto file_t::path() const -> const std::string& {
    return inner->path();
}

auto file_t::emit(const record_t& record, const string_view& formatted) -> void {
    const auto filename = inner->filename(record);

    std::lock_guard<std::mutex> lock(inner->mutex);
    inner->backend(filename).write(formatted);
}

}  // namespace sink

namespace experimental {

builder<sink::file_t>::builder(const std::string& filename) :
    properties(new sink::file_properties_t{filename, 0}, deleter_t())
{}

auto builder<sink::file_t>::interval(std::size_t count) -> builder& {
    properties->interval = count;
    return *this;
}

auto builder<sink::file_t>::build() && -> std::unique_ptr<sink_t> {
    return std::unique_ptr<sink_t>(new sink::file_t(*properties));
}

auto factory<sink::file_t>::type() const noexcept -> const char* {
    return "file";
}

auto factory<sink::file_t>::from(const config::node_t& config) const -> std::unique_ptr<sink_t> {
    const auto filename = config["path"].to_string();

    if (!filename) {
        throw std::invalid_argument("field 'path' is required");
    }

    builder<sink::file_t> builder(filename.get());

    if (auto flush = config["flush"]) {
        if (auto value = flush.to_uint64()) {
            builder.interval(value.get());
        }
    }

    return std::move(builder).build();
}

}  // namespace experimental

template auto deleter_t::operator()(sink::file_properties_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
