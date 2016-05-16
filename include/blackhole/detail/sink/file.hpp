#pragma once

#include <fstream>
#include <limits>
#include <map>
#include <mutex>

#include <boost/assert.hpp>

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/file.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class file_properties_t {
public:
    std::string filename;
    std::size_t interval;
};

class backend_t;
class flusher_t;

class backend_factory_t {
public:
    virtual ~backend_factory_t() = default;
    virtual auto create(const std::string& path) const -> std::unique_ptr<backend_t> = 0;
};

class flusher_factory_t {
public:
    virtual ~flusher_factory_t() = default;
    virtual auto create() const -> std::unique_ptr<flusher_t> = 0;
};

namespace file {

struct backend_t {
    std::size_t counter;
    std::size_t interval;
    std::unique_ptr<std::ofstream> stream;

    backend_t(const std::string& filename, std::size_t interval) :
        counter(0),
        interval(interval),
        stream(new std::ofstream)
    {
        BOOST_ASSERT(interval > 0);

        stream->exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            stream->open(filename, std::ios::app);
        } catch (const std::system_error& err) {
            // Transform unspecified ios category into the system one to obtain readable message,
            // otherwise there will be completely weird error reason.
            throw std::system_error(err.code().value(), std::system_category());
        } catch (const std::exception&) {
            throw std::system_error(errno, std::system_category());
        }
    }

    auto write(const string_view& message) -> void {
        stream->write(message.data(), static_cast<std::streamsize>(message.size()));
        stream->put('\n');

        counter = (counter + 1) % interval;

        if (counter == 0) {
            flush();
        }
    }

    auto flush() -> void {
        stream->flush();
    }
};

class inner_t {
    struct {
        std::string filename;
        std::size_t interval;
        std::map<std::string, backend_t> backends;
    } data;

public:
    std::mutex mutex;

    inner_t(std::string filename, std::size_t interval) {
        data.filename = std::move(filename);
        data.interval = interval > 0 ? interval : std::numeric_limits<std::size_t>::max();
    }

    virtual ~inner_t() {}

    auto path() const noexcept -> const std::string& {
        return data.filename;
    }

    auto interval() const noexcept -> std::size_t {
        return data.interval;
    }

    virtual auto filename(const record_t&) const -> std::string {
        // TODO: Generate path from tokens, for now just return static path.
        return {data.filename};
    }

    virtual auto backend(const std::string& filename) -> backend_t& {
        const auto it = data.backends.find(filename);

        if (it == data.backends.end()) {
            return data.backends.insert(it, std::make_pair(filename, backend_t(filename, interval())))->second;
        }

        return it->second;
    }
};

}  // namespace file

class file_t : public sink_t {
    std::unique_ptr<file::inner_t> inner;

public:
    /// Constructs a file sink, which will write all incoming events to the file or files located at
    /// the specified path.
    ///
    /// The path can contain attribute placeholders, meaning that the real destination name will be
    /// deduced at runtime using provided log record. No real file will be opened at construction
    /// time.
    /// The file is opened by default in append mode meaning seek to the end of stream immediately
    /// after open.
    ///
    /// \param path a path with final destination file to open. All files are opened with append
    ///     mode by default.
    /// \note associated files will be opened on demand during the first write operation.
    explicit file_t(const std::string& path);

    file_t(const file_properties_t& properties);

    // TODO: DI.
    // file_t(const std::string& path,
    //        std::unique_ptr<backend_factory_t> backend_factory,
    //        std::unique_ptr<flusher_factory_t> flusher_factory);

    /// Returns a const lvalue reference to destination path pattern.
    ///
    /// The path can contain attribute placeholders, meaning that the real destination name will be
    /// deduced at runtime using provided log record. No real file will be opened at construction
    /// time.
    auto path() const -> const std::string&;

    /// Outputs the formatted message with its associated record to the file.
    ///
    /// Depending on the filename pattern it is possible to write into multiple destinations.
    auto emit(const record_t& record, const string_view& formatted) -> void;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
