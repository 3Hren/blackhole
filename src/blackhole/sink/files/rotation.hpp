#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "blackhole/config.hpp"

#include "blackhole/sink/files/rotation/config.hpp"
#include "blackhole/sink/files/rotation/counter.hpp"
#include "blackhole/sink/files/rotation/naming/filter.hpp"
#include "blackhole/sink/files/rotation/timer.hpp"
#include "blackhole/sink/files/rotation/watcher.hpp"

BLACKHOLE_BEG_NS

namespace sink {

//! Tag class for file sinks with no rotation policy.
class null_rotator_t;

template<class Backend, class Watcher, class TimePicker = rotation::timepicker_t>
class rotator_t {
public:
    typedef Backend backend_type;
    typedef Watcher watcher_type;
    typedef TimePicker timepicker_type;
    typedef rotation::config_t<watcher_type> config_type;

private:
    config_type config;
    watcher_type watcher;
    backend_type& backend;

    rotation::naming::basename_t generator;
    rotation::counter_t counter;

public:
    static const char* name() {
        return "rotate";
    }

    rotator_t(const config_type& config, backend_type& backend) :
        config(config),
        watcher(config.watcher),
        backend(backend),
        generator(config.pattern),
        counter(
            rotation::counter_t::from_string(
                boost::algorithm::replace_all_copy(
                    config.pattern,
                    "%(filename)s",
                    backend.filename()
                )
            )
        )
    {}

    bool necessary(const std::string& message) const {
        return watcher(backend, message);
    }

    void rotate() const {
        backend.flush();
        backend.close();
        rollover();
        backend.open();
    }

private:
    void rollover() const {
        const std::string& filename = backend.filename();
        const std::string& basename = generator.transform(filename);

        if (counter.valid()) {
            rollover(backend.listdir(), basename);
        }

        if (backend.exists(filename)) {
            backend.rename(filename, backup_filename(basename));
        }
    }

    void rollover(std::vector<std::string> filenames, const std::string& pattern) const {
        filenames.erase(std::remove_if(filenames.begin(),
                                       filenames.end(),
                                       rotation::naming::filter_t(pattern)),
                        filenames.end());
        std::sort(filenames.begin(), filenames.end(),
                  rotation::naming::comparator::time::descending<Backend>(backend));
        if (filenames.size() > static_cast<std::size_t>(config.backups - 1)) {
            filenames.erase(filenames.begin() + config.backups - 1, filenames.end());
        }

        for (int n = filenames.size() - 1; n >= 0; --n) {
            const std::string& filename = filenames.at(n);
            if (backend.exists(filename)) {
                backend.rename(filename, counter.next(filename, n + 1));
            }
        }
    }

    std::string backup_filename(const std::string& pattern) const {
        std::string filename = pattern;
        boost::algorithm::replace_all(filename, "%N", "1");
        std::time_t time = timepicker_type::current();
        char buf[128];
        if (strftime(buf, 128, filename.data(), std::gmtime(&time)) == 0) {
            // Do nothing.
        }

        return std::string(buf);
    }
};

template<class Backend>
class rotator_t<Backend, rotation::watcher::move_t, rotation::timepicker_t> {
public:
    typedef Backend backend_type;
    typedef rotation::watcher::move_t watcher_type;
    typedef rotation::config_t<watcher_type> config_type;

private:
    watcher_type watcher;
    backend_type& backend;

public:
    static const char* name() {
        return "rotate";
    }

    rotator_t(const config_type& config, Backend& backend) :
        watcher(config.watcher),
        backend(backend)
    {}

    bool necessary(const std::string& message) const {
        return watcher(backend, message);
    }

    void rotate() const {
        backend.flush();
        backend.close();
        backend.open();
    }
};

} // namespace sink

BLACKHOLE_END_NS
