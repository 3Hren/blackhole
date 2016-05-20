#pragma once

#include <fstream>
#include <limits>
#include <map>
#include <mutex>

#include <boost/assert.hpp>

#include "blackhole/cpp17/string_view.hpp"
#include "blackhole/sink.hpp"
#include "blackhole/sink/file.hpp"

#include "blackhole/detail/memory.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class file_properties_t {
public:
    std::string filename;
    std::size_t interval;
};

namespace file {

/// Flush suggest policy.
class flusher_t {
public:
    enum result_t {
        /// No flush required.
        idle,
        /// It's time to flush.
        flush
    };

public:
    virtual ~flusher_t() = default;

    /// Resets the current flusher state.
    virtual auto reset() -> void = 0;

    /// Updates the flusher, incrementing its counters.
    ///
    /// \param nwritten bytes consumed during previous write operation.
    virtual auto update(std::size_t nwritten) -> result_t = 0;
};

class repeat_flusher_t : public flusher_t {
public:
    typedef std::size_t threshold_type;

private:
    threshold_type counter;
    threshold_type threshold_;

public:
    constexpr repeat_flusher_t(threshold_type threshold) noexcept :
        counter(0),
        threshold_(threshold)
    {}

    auto threshold() const noexcept -> threshold_type {
        return threshold_;
    }

    auto count() const noexcept -> threshold_type {
        return counter;
    }

    auto reset() -> void override {
        counter = 0;
    }

    auto update(std::size_t nwritten) -> flusher_t::result_t override {
        if (nwritten != 0) {
            counter = (counter + 1) % threshold();

            if (counter == 0) {
                return flusher_t::flush;
            }
        }

        return flusher_t::idle;
    }
};

class bytecount_flusher_t : public flusher_t {
public:
    typedef std::uint64_t threshold_type;

private:
    threshold_type counter;
    threshold_type threshold_;

public:
    constexpr bytecount_flusher_t(threshold_type threshold) noexcept :
        counter(0),
        threshold_(threshold)
    {}

    auto threshold() const noexcept -> threshold_type {
        return threshold_;
    }

    auto count() const noexcept -> threshold_type {
        return counter;
    }

    auto reset() -> void override {
        counter = 0;
    }

    auto update(std::size_t nwritten) -> flusher_t::result_t override {
        auto result = flusher_t::result_t::idle;

        if (nwritten != 0) {
            if (counter + nwritten >= threshold()) {
                result = flusher_t::result_t::flush;
            }

            counter = (counter + nwritten) % threshold();
        }

        return result;
    }
};

// class stream_factory_t {
// public:
//     virtual ~stream_factory_t() = default;
//     virtual auto create(const std::string& path) const -> std::unique_ptr<std::ostream> = 0;
// };

class flusher_factory_t {
public:
    virtual ~flusher_factory_t() = default;
    virtual auto create() const -> std::unique_ptr<flusher_t> = 0;
};

class repeat_flusher_factory_t : public flusher_factory_t {
public:
    typedef typename repeat_flusher_t::threshold_type threshold_type;

private:
    threshold_type value;

public:
    explicit repeat_flusher_factory_t(threshold_type threshold) noexcept;

    auto threshold() const noexcept -> threshold_type;
    auto create() const -> std::unique_ptr<flusher_t> override;
};

class bytecount_flusher_factory_t : public flusher_factory_t {
public:
    typedef typename bytecount_flusher_t::threshold_type threshold_type;

private:
    threshold_type value;

public:
    explicit bytecount_flusher_factory_t(threshold_type threshold) noexcept;

    auto threshold() const noexcept -> threshold_type;
    auto create() const -> std::unique_ptr<flusher_t> override;
};

auto parse_dunit(const std::string& encoded) -> std::uint64_t;

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
        } catch (...) {
            throw std::system_error(errno, std::system_category());
        }
    }

    auto write(const string_view& message) -> void {
        /// stream->write(message);
        /// stream->put('\n');
        /// if (flusher->trigger() == flusher_t::flush) {
        ///     stream->flush();
        /// }
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

}  // namespace file

class file_t : public sink_t {
    struct {
        std::string filename;
        std::size_t interval;
        std::map<std::string, file::backend_t> backends;
    } data;

    mutable std::mutex mutex;

public:
    /// \param path a path with final destination file to open. All files are opened with append
    ///     mode by default.
    /// \param interval flush interval in number of write operations.
    explicit file_t(const std::string& path, std::size_t interval = 0);

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

    auto interval() const noexcept -> std::size_t;

    auto filename(const record_t&) const -> std::string;

    auto backend(const std::string& filename) -> file::backend_t&;

    /// Outputs the formatted message with its associated record to the file.
    ///
    /// Depending on the filename pattern it is possible to write into multiple destinations.
    auto emit(const record_t& record, const string_view& formatted) -> void override;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
