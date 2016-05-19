#include "blackhole/sink/file.hpp"

#include <cctype>

#include <boost/lexical_cast.hpp>
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
namespace file {

auto parse_interval(const std::string& encoded) -> std::uint64_t {
    const auto ipos = std::find_if(std::begin(encoded), std::end(encoded), [&](char c) -> bool {
        return !std::isdigit(c);
    });

    if (ipos == std::end(encoded)) {
        return boost::lexical_cast<std::uint64_t>(encoded);
    }

    const auto pos = static_cast<std::size_t>(std::distance(std::begin(encoded), ipos));
    const auto base = boost::lexical_cast<std::uint64_t>(encoded.substr(0, pos));
    const auto unit = encoded.substr(pos);

    const std::map<std::string, std::uint64_t> mapping {
        {"B",  1   },
        {"MB", 1000},
    };

    const auto it = mapping.find(unit);
    if (it == std::end(mapping)) {
        throw std::invalid_argument("unknown data unit - " + unit);
    }

    return base * it->second;
}

}  // namespace file

file_t::file_t(const std::string& filename, std::size_t interval) {
    data.filename = filename;
    data.interval = interval > 0 ? interval : std::numeric_limits<std::size_t>::max();
}

file_t::file_t(const file_properties_t& properties) :
    file_t(properties.filename, properties.interval)
{}

auto file_t::path() const -> const std::string& {
    return data.filename;
}

auto file_t::interval() const noexcept -> std::size_t {
    return data.interval;
}

auto file_t::filename(const record_t&) const -> std::string {
    // TODO: Generate path from tokens, for now just return static path.
    return {data.filename};
}

auto file_t::backend(const std::string& filename) -> file::backend_t& {
    const auto it = data.backends.find(filename);

    if (it == data.backends.end()) {
        return data.backends.insert(it, std::make_pair(filename, file::backend_t(filename, interval())))->second;
    }

    return it->second;
}

auto file_t::emit(const record_t& record, const string_view& formatted) -> void {
    const auto filename = this->filename(record);

    std::lock_guard<std::mutex> lock(mutex);
    backend(filename).write(formatted);
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
