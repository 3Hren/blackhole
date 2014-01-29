#pragma once

#include <cstdint>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "blackhole/sink/files/rotation/config.hpp"
#include "blackhole/sink/files/rotation/timer.hpp"
#include "blackhole/sink/files/rotation/naming.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace sink {

//! Tag for file sinks with no rotation.
template<class Backend, class Timer = timer_t> class NoRotation;

template<class Backend, class Timer = timer_t>
class rotator_t {
    rotation::config_t config;
    Backend& backend;
    Timer m_timer;
    basename::generator_t generator;
public:
    static const char* name() {
        return "rotate";
    }

    rotator_t(Backend& backend) :
        backend(backend)
    {}

    rotator_t(const rotation::config_t& config, Backend& backend) :
        config(config),
        backend(backend),
        generator(config.pattern)
    {}

    Timer& timer() {
        return m_timer;
    }

    bool necessary() const {
        // In case of size based rotation everything is simple: file.size() + message.size() >= size;
        // In case of datetime based rotation that's seems to be a bit difficult: get_current_time() >= current_fence().
        return false;
    }

    void rotate() const {
        backend.flush();
        backend.close();
        rollover();
        backend.open();
    }

private:
    struct rollover_pair_t {
        const std::string current;
        const std::string renamed;
    };

    void rollover() const {
        const std::string& filename = backend.filename();
        const std::string& basename = generator.basename(filename);

        rollover(backend.listdir(), basename);

        if (backend.exists(filename)) {
            backend.rename(filename, backup_filename(basename));
        }
    }

    void rollover(std::vector<std::string> filenames, const std::string& pattern) const {
        filter(&filenames, matching::datetime_t(pattern, config.backups));
        std::sort(filenames.begin(), filenames.end(), comparator::time::ascending<Backend>(backend));

        std::vector<rollover_pair_t> pairs = make_rollover_pairs(filenames, pattern, config.backups);
        for (auto it = pairs.begin(); it != pairs.end(); ++it) {
            const rollover_pair_t& pair = *it;
            if (backend.exists(pair.current)) {
                backend.rename(pair.current, pair.renamed);
            }
        }
    }

    template<typename Filter>
    void filter(std::vector<std::string>* filenames, Filter filter) const {
        filenames->erase(std::remove_if(filenames->begin(), filenames->end(), filter), filenames->end());
    }

    std::vector<rollover_pair_t>
    make_rollover_pairs(const std::vector<std::string>& filenames, const std::string& pattern, int backups) const {
        std::vector<rollover_pair_t> result;

        int pos = matching::counting::pos(pattern);
        if (pos == -1) {
            return result;
        }

        int counter = 0;
        for (auto it = filenames.begin(); it != filenames.end() && counter < backups; ++it, ++counter) {
            const std::string& current = *it;

            std::string current_id = std::string(current.begin() + pos, current.begin() + pos + 1);
            int id = boost::lexical_cast<int>(current_id);
            id++;
            std::string renamed = current;
            renamed.replace(pos, 1, boost::lexical_cast<std::string>(id));
            result.push_back({ current, renamed });
        }

        return result;
    }

    std::string backup_filename(const std::string& pattern) const {
        std::string filename = pattern;
        boost::algorithm::replace_all(filename, "%N", "1");
        std::time_t time = m_timer.current();
        char buf[128];
        if (strftime(buf, 128, filename.data(), std::gmtime(&time)) == 0) {
            // Do nothing.
        }

        return std::string(buf);
    }
};

} // namespace sink

} // namespace blackhole
