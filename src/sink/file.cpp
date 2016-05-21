#include "blackhole/sink/file.hpp"

#include <cctype>
#include <fstream>
#include <ostream>

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>

#include "blackhole/config/node.hpp"
#include "blackhole/config/option.hpp"
#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/record.hpp"

#include "blackhole/detail/sink/file.hpp"
#include "blackhole/detail/sink/file/flusher/bytecount.hpp"
#include "blackhole/detail/sink/file/flusher/repeat.hpp"
#include "blackhole/detail/sink/file/stream.hpp"
#include "blackhole/detail/util/deleter.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace flusher {

repeat_factory_t::repeat_factory_t(threshold_type threshold) noexcept :
    value(threshold)
{}

auto repeat_factory_t::threshold() const noexcept -> threshold_type {
    return value;
}

auto repeat_factory_t::create() const -> std::unique_ptr<flusher_t> {
    return blackhole::make_unique<repeat_t>(threshold());
}

bytecount_factory_t::bytecount_factory_t(threshold_type threshold) noexcept :
    value(threshold)
{}

auto bytecount_factory_t::threshold() const noexcept -> threshold_type {
    return value;
}

auto bytecount_factory_t::create() const -> std::unique_ptr<flusher_t> {
    return blackhole::make_unique<bytecount_t>(threshold());
}

auto parse_dunit(const std::string& encoded) -> std::uint64_t {
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
        {"B",  1},
        {"kB", 1e3},
        {"MB", 1e6},
        {"GB", 1e9},
        {"KiB", 1ULL << 10},
        {"MiB", 1ULL << 20},
        {"GiB", 1ULL << 30},
    };

    const auto it = mapping.find(unit);
    if (it == std::end(mapping)) {
        throw std::invalid_argument("unknown data unit - " + unit);
    }

    return base * it->second;
}

}  // namespace flusher

auto ofstream_factory_t::create(const std::string& filename, std::ios_base::openmode mode) const ->
    std::unique_ptr<std::ostream>
{
    auto stream = blackhole::make_unique<std::ofstream>();
    stream->exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        stream->open(filename, mode);
    } catch (const std::system_error& err) {
        // Transform unspecified ios category into the system one to be able to obtain readable
        // error message instead completely weird description.
        throw std::system_error(err.code().value(), std::system_category());
    } catch (...) {
        throw std::system_error(errno, std::system_category());
    }

    // This hack is needed to trick std::unique_ptr behavior, which is unable to implicitly convert
    // covariant types because of strongly typed deleter.
    return std::unique_ptr<std::ostream>(stream.release());
}

}  // namespace file

file_t::file_t(const std::string& path,
               std::unique_ptr<file::stream_factory_t> stream_factory,
               std::unique_ptr<file::flusher_factory_t> flusher_factory) :
    stream_factory(std::move(stream_factory)),
    flusher_factory(std::move(flusher_factory))
{
    data.path = path;
}

auto file_t::path() const -> const std::string& {
    return data.path;
}

auto file_t::filename(const record_t&) const -> std::string {
    // TODO: Generate path from tokens, for now just return static path.
    return {data.path};
}

auto file_t::backend(const std::string& filename) -> file::backend_t& {
    const auto it = data.backends.find(filename);

    if (it == data.backends.end()) {
        auto stream = stream_factory->create(filename, std::ios_base::app);
        auto flusher = flusher_factory->create();

        return data.backends.insert(it,
            std::make_pair(filename, file::backend_t(std::move(stream), std::move(flusher))))->second;
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

class builder<sink::file_t>::inner_t {
public:
    std::string filename;
    std::unique_ptr<sink::file::flusher_factory_t> ffactory;
};

builder<sink::file_t>::builder(const std::string& path) :
    p(new inner_t{path, nullptr}, deleter_t())
{
    p->ffactory = blackhole::make_unique<sink::file::flusher::repeat_factory_t>(std::size_t(0));
}

auto builder<sink::file_t>::flush_every(bytes_t bytes) & -> builder& {
    p->ffactory = blackhole::make_unique<sink::file::flusher::bytecount_factory_t>(bytes.count());
    return *this;
}

auto builder<sink::file_t>::flush_every(bytes_t bytes) && -> builder&& {
    return std::move(flush_every(bytes));
}

auto builder<sink::file_t>::flush_every(std::size_t events) & -> builder& {
    p->ffactory = blackhole::make_unique<sink::file::flusher::repeat_factory_t>(events);
    return *this;
}

auto builder<sink::file_t>::flush_every(std::size_t events) && -> builder&& {
    return std::move(flush_every(events));
}

auto builder<sink::file_t>::build() && -> std::unique_ptr<sink_t> {
    return blackhole::make_unique<sink::file_t>(
        std::move(p->filename),
        blackhole::make_unique<sink::file::ofstream_factory_t>(),
        std::move(p->ffactory));
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
            builder.flush_every(value.get());
        }

        if (auto value = flush.to_string()) {
            // builder.threshold(value.get());
        }
    }

    return std::move(builder).build();
}

}  // namespace experimental

template auto deleter_t::operator()(experimental::builder<sink::file_t>::inner_t* value) -> void;

}  // namespace v1
}  // namespace blackhole
