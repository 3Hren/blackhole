#pragma once

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "blackhole/sink/files/rotation/config.hpp"
#include "blackhole/sink/files/rotation/timer.hpp"
#include "blackhole/sink/files/rotation/naming.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

struct counter_t {
    std::string prefix;
    std::string suffix;
    uint width;

    counter_t(const std::string& prefix, const std::string& suffix, uint width) :
        prefix(std::move(prefix)),
        suffix(std::move(suffix)),
        width(width)
    {}

    counter_t(std::string&& prefix, std::string&& suffix, uint width) :
        prefix(std::move(prefix)),
        suffix(std::move(suffix)),
        width(width)
    {}

    bool operator ==(const counter_t& other) const {
        return prefix == other.prefix && suffix == other.suffix && width == other.width;
    }

    friend std::ostream& operator <<(std::ostream& stream, const counter_t& counter) {
        stream << "counter_t('" << counter.prefix << "', '" << counter.suffix << "', " << counter.width << ")";
        return stream;
    }

    bool valid() const {
        return width != 0;
    }

    //! I just leave these examples as mini documentation.
    //! pattern:     test.log.%Y%m%d.%N      %(filename).%N      %(filename).%Y%m%d.%N
    //! basename:    test.log.%Y%m%d.%N      test.log.%N         test.log.%Y%m%d.%N
    //! current:     test.log.20140130.1     test.log.1          test.log.20140130.1
    //! newname:     test.log.20140130.2     test.log.2          test.log.20140130.2
    std::string next(const std::string& filename) const {
        BOOST_ASSERT(filename.size() - prefix.size() - suffix.size() >= 0);
        const std::string& counter = filename.substr(prefix.size(),
                                                     filename.size() - prefix.size() - suffix.size());
        uint value = 0;
        try {
            value = boost::lexical_cast<uint>(counter);
        } catch (const boost::bad_lexical_cast&) {
            // Eat this.
        }

        std::ostringstream stream;
        stream << std::string(filename.begin(), filename.begin() + prefix.size())
               << std::setfill('0') << std::setw(width) << (value + 1)
               << std::string(filename.begin() + prefix.size() + std::max(width, sink::matching::digits(value)), filename.end());
        return stream.str();
    }

    static counter_t from_string(const std::string& pattern) {
        std::string prefix;
        std::string suffix;
        int width = 0;
        bool found = false;
        for (auto it = pattern.begin(); it != pattern.end(); ++it) {
            if (found) {
                suffix.push_back(*it);
            } else if (*it == '%') {
                it++;
                std::string value;

                for (; it != pattern.end(); ++it) {
                    if (*it == 'N') {
                        found = true;
                        break;
                    }

                    value.push_back(*it);
                    if (!std::isdigit(*it)) {
                        break;
                    }
                }

                if (found) {
                    width = value.empty() ? 1 : boost::lexical_cast<uint>(value);
                } else {
                    prefix.push_back('%');
                    prefix.append(value);
                }
            } else {
                prefix.push_back(*it);
            }
        }

        boost::replace_all(prefix, "%Y", "YYYY");
        boost::replace_all(prefix, "%m", "mm");
        boost::replace_all(prefix, "%d", "dd");
        boost::replace_all(prefix, "%H", "HH");
        boost::replace_all(prefix, "%M", "MM");
        boost::replace_all(prefix, "%s", "ss");

        boost::replace_all(suffix, "%Y", "YYYY");
        boost::replace_all(suffix, "%m", "mm");
        boost::replace_all(suffix, "%d", "dd");
        boost::replace_all(suffix, "%H", "HH");
        boost::replace_all(suffix, "%M", "MM");
        boost::replace_all(suffix, "%s", "ss");

        return counter_t(std::move(prefix), std::move(suffix), width);
    }

    int count(const std::string& str, const std::string& obj ) {
        int n = 0;
        std::string::size_type pos = 0;
        while ((pos = obj.find(str, pos)) != std::string::npos) {
            n++;
            pos += str.size();
        }

        return n;
    }
};

namespace sink {

//! Tag for file sinks with no rotation.
template<class Backend, class Timer = timer_t> class NoRotation;

template<class Backend, class Timer = timer_t>
class rotator_t {
    rotation::config_t config;
    Backend& backend;
    Timer m_timer;
    basename::generator_t generator;

    counter_t counter;
public:
    static const char* name() {
        return "rotate";
    }

    rotator_t(const rotation::config_t& config, Backend& backend) :
        config(config),
        backend(backend),
        generator(config.pattern),
        counter(counter_t::from_string(config.pattern))
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
        std::string current;
        std::string renamed;
    };

    void rollover() const {
        const std::string& filename = backend.filename();
        const std::string& basename = generator.basename(filename);

        if (counter.valid()) {
            rollover(backend.listdir(), basename);
        }

        if (backend.exists(filename)) {
            backend.rename(filename, backup_filename(basename));
        }
    }

    void rollover(std::vector<std::string> filenames, const std::string& pattern) const {
        filter(&filenames, matching::datetime_t(pattern));
        std::sort(filenames.begin(), filenames.end(), comparator::time::ascending<Backend>(backend));
        for (auto it = filenames.begin(); it != filenames.end(); ++it) {
            const std::string& filename = *it;
            if (backend.exists(filename)) {
                backend.rename(filename, counter.next(filename));
            }
        }
    }

    template<typename Filter>
    void filter(std::vector<std::string>* filenames, Filter filter) const {
        filenames->erase(std::remove_if(filenames->begin(), filenames->end(), filter), filenames->end());
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
