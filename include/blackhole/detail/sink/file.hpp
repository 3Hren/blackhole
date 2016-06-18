#pragma once

#include <fstream>
#include <limits>
#include <map>
#include <mutex>

#include <boost/assert.hpp>

#include "blackhole/stdext/string_view.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/file.hpp"

#include "blackhole/detail/memory.hpp"

#include "file/flusher.hpp"
#include "file/stream.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {

class backend_t {
    std::unique_ptr<std::ostream> stream;
    std::unique_ptr<flusher_t> flusher;

public:
    backend_t(std::unique_ptr<std::ostream> stream, std::unique_ptr<flusher_t> flusher) :
        stream(std::move(stream)),
        flusher(std::move(flusher))
    {}

    auto write(const string_view& message) -> void {
        stream->write(message.data(), static_cast<std::streamsize>(message.size()));
        stream->put('\n');
        if (flusher->update(message.size() + 1) == flusher_t::flush) {
            stream->flush();
        }
    }
};

}  // namespace file

class file_t : public sink_t {
    std::unique_ptr<file::stream_factory_t> stream_factory;
    std::unique_ptr<file::flusher_factory_t> flusher_factory;

    struct {
        std::string path;
        std::map<std::string, file::backend_t> backends;
    } data;

    mutable std::mutex mutex;

public:
    /// \param path a path with final destination file to open. All files are opened with append
    ///     mode by default.
    file_t(const std::string& path,
           std::unique_ptr<file::stream_factory_t> stream_factory,
           std::unique_ptr<file::flusher_factory_t> flusher_factory);

    /// Returns a const lvalue reference to destination path pattern.
    ///
    /// The path can contain attribute placeholders, meaning that the real destination name will be
    /// deduced at runtime using provided log record. No real file will be opened at construction
    /// time.
    auto path() const -> const std::string&;

    auto filename(const record_t& record) const -> std::string;

    auto backend(const std::string& filename) -> file::backend_t&;

    /// Outputs the formatted message with its associated record to the file.
    ///
    /// Depending on the filename pattern it is possible to write into multiple destinations.
    auto emit(const record_t& record, const string_view& formatted) -> void override;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
