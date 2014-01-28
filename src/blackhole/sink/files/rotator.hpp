#pragma once

#include <cstdint>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace sink {

struct timer_t {
    std::time_t current() const {
        return std::time(nullptr);
    }
};

//! Tag for file sinks with no rotation.
template<class Backend, class Timer = timer_t> class NoRotation;

namespace rotator {

struct config_t {
    std::string pattern;
    std::uint16_t backups;
    std::uint64_t size;

    config_t(const std::string& pattern = ".%N", std::uint16_t backups = 5, std::uint64_t size = 10 * 1024 * 1024) :
        pattern(pattern),
        backups(backups),
        size(size)
    {}
};

} // namespace rotator

namespace time {

template<typename Backend>
struct ascending {
    Backend& backend;

    ascending(Backend& backend) : backend(backend) {}

    bool operator ()(const std::string& lhs, const std::string& rhs) const {
        return backend.changed(lhs) < backend.changed(rhs);
    }
};

}

namespace matcher {

struct datetime_t {
    const std::string& pattern;
    const std::uint16_t backups;

    datetime_t(const std::string& pattern, std::uint16_t backups) :
        pattern(pattern),
        backups(backups)
    {}

    bool operator ()(const std::string& filename) const {
        return !match(filename);
    }

    bool match(const std::string& filename) const {
        auto f_it = filename.begin();
        auto f_end = filename.end();
        auto p_it = pattern.begin();
        auto p_end = pattern.end();

        bool placeholder_expected = false;
        while (f_it != filename.end() && p_it != pattern.end()) {
            char f_c = *f_it;
            char p_c = *p_it;

            if (!placeholder_expected) {
                if (p_c == '%') {
                    placeholder_expected = true;
                    p_it++;
                } else if (f_c == p_c) {
                    f_it++;
                    p_it++;
                } else {
                    return false;
                }
            } else {
                switch (p_c) {
                case '%':
                    if (p_c == f_c) {
                        ++p_it;
                        ++f_it;
                        break;
                    } else {
                        return false;
                    }
                case 'M':
                case 'H':
                case 'd':
                case 'm':
                    if (!scan_digits(f_it, f_end, 2)) {
                        return false;
                    }
                    ++p_it;
                    break;
                case 'Y':
                    if (!scan_digits(f_it, f_end, 4)) {
                        return false;
                    }
                    ++p_it;
                    break;
                case 'N':
                    if (!scan_digits(f_it, f_end, digits(backups))) {
                        return false;
                    }
                    ++p_it;
                    break;
                default:
                    throw std::invalid_argument("Unsupported placeholder used in pattern for file scanning");
                }

                placeholder_expected = false;
            }
        }

        if (p_it == p_end) {
            if (f_it != f_end) {
                return scan_digits(f_it, f_end, std::distance(f_it, f_end));
            } else {
                return true;
            }
        } else {
            return false;
        }
    }

private:
    static bool scan_digits(std::string::const_iterator& it, std::string::const_iterator end, int n) {
        for (; n > 0; --n) {
            if (it == end) {
                return false;
            }

            const char c = *it++;
            if (!std::isdigit(c)) {
                return false;
            }
        }

        return true;
    }

    static uint digits(uint number) {
        uint digits = 0;
        while (number) {
            number /= 10;
            digits++;
        }

        return digits;
    }
};

} // namespace match

template<class Backend, class Timer = timer_t>
class rotator_t {
    rotator::config_t config;
    Backend& backend;
    Timer m_timer;
public:
    static const char* name() {
        return "rotate";
    }

    rotator_t(Backend& backend) :
        backend(backend)
    {}

    rotator_t(const rotator::config_t& config, Backend& backend) :
        config(config),
        backend(backend)
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

        const std::string& filename = backend.filename();

        std::string pattern = config.pattern;
        if (config.pattern.find("%(filename)s") != std::string::npos) {
            boost::algorithm::replace_all(pattern, "%(filename)s", filename);
        }

        rotate(backend.listdir(), pattern);

        if (backend.exists(filename)) {
            backend.rename(filename, format(pattern));
        }

        backend.open();
    }

private:
    void rotate(std::vector<std::string> filenames, const std::string& pattern) const {
        filter(&filenames, matcher::datetime_t(pattern, config.backups));
        std::sort(filenames.begin(), filenames.end(), time::ascending<Backend>(backend));

        std::vector<std::pair<std::string, std::string>> pairs = cumilative(filenames, pattern, config.backups);

        for (auto it = pairs.begin(); it != pairs.end(); ++it) {
            const std::pair<std::string, std::string>& pair = *it;
            if (backend.exists(pair.first)) {
                backend.rename(pair.first, pair.second);
            }
        }
    }

    template<typename Filter>
    void filter(std::vector<std::string>* filenames, Filter filter) const {
        filenames->erase(std::remove_if(filenames->begin(), filenames->end(), filter), filenames->end());
    }

    std::vector<std::pair<std::string, std::string> >
    cumilative(const std::vector<std::string>& filenames, const std::string& pattern, int backups) const {
        std::vector<std::pair<std::string, std::string>> result;
        int pos = pattern.find("%N");
        if (pos == -1) {
            return result;
        }

        bool placeholder_expected = false;
        for (auto it = pattern.begin(); it != pattern.end() && it != pattern.begin() + pos; ++it) {
            char c = *it;

            if (!placeholder_expected) {
                if (c == '%') {
                    placeholder_expected = true;
                }
            } else {
                switch (c) {
                case 'M':
                case 'H':
                case 'd':
                case 'm':
                case 'N':
                    break;
                case 'Y':
                    pos += 2;
                    break;
                default:
                    throw std::invalid_argument(utils::format("Unsupported placeholder used in pattern for file scanning: %s", c));
                }

                placeholder_expected = false;
            }
        }

        int counter = 0;
        for (auto it = filenames.begin(); it != filenames.end(); ++it) {
            const std::string& filename = *it;
            if (counter >= backups) {
                break;
            }

            std::string fn = filename;
            std::string current = std::string(fn.begin() + pos, fn.begin() + pos + 1);
            int i = atoi(current.data());
            i++;
            fn.replace(pos, 1, boost::lexical_cast<std::string>(i));
            result.push_back(std::make_pair(filename, fn));
            counter++;
        }
        return result;
    }

    std::string format(const std::string& pattern) const {
        std::string filename = pattern;
        boost::algorithm::replace_all(filename, "%N", "1");
        std::time_t time = m_timer.current();
        char buf[128];
        strftime(buf, 128, filename.data(), std::gmtime(&time));
        return buf;
    }
};

} // namespace sink

} // namespace blackhole
