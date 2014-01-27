#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include <boost/algorithm/string.hpp>

#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace sink {

//! Tag for file sinks with no rotation.
template<typename Backend> class NoRotation;

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

}

template<typename Backend>
class rotator_t {
    rotator::config_t config;
    Backend& backend;
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

        std::cout << utils::format("Pattern: %s", pattern) << std::endl;

        // Get list of files in the directory
        std::vector<std::string> files = backend.listdir();
        std::cout << utils::format("List: [%s]", boost::join(files, ", ")) << std::endl;

        // Files are filtered by pattern. Only matched files left.
        files = filter(files, pattern);
        std::cout << utils::format("Filtered: [%s]", boost::join(files, ", ")) << std::endl;

        // Files are sorted by timestamp.
        sort(files);
        std::cout << utils::format("Sorted: [%s]", boost::join(files, ", ")) << std::endl;

        // Maximum `backups` pairs are collected.
        std::vector<std::pair<std::string, std::string>> pairs = cumilative(files, pattern, config.backups);
        if (pairs.empty()) {
            std::cout << "No pairs." << std::endl;
        } else {
            std::cout << "Pairs:" << std::endl;
            for (auto pair : pairs) std::cout << pair.first << " -> " << pair.second << std::endl;
        }

        // Rotating pairs. Counter (if present) is increasing.
        for (auto pair : pairs) {
            if (backend.exists(pair.first)) {
                backend.rename(pair.first, pair.second);
            }
        }

        if (backend.exists(filename)) {
            std::cout << utils::format("%s -> %s", filename, format(pattern)) << std::endl;
            backend.rename(filename, format(pattern));
        }

        backend.open();
    }

    std::vector<std::string> filter(const std::vector<std::string>& filenames, const std::string& pattern) const {
        std::vector<std::string> result;
        for (auto filename : filenames) {
//            std::cout << utils::format("%s: %s", filename, match(filename, pattern)) << std::endl;
            if (match(filename, pattern)) {
                result.push_back(filename);
            }
        }

        return result;
    }

    bool match(const std::string& filename, const std::string& pattern) const {
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
                    if (!scan_digits(f_it, f_end, digits(config.backups))) {
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
                // The actual file name may end with an additional counter
                // that is added by the collector in case if file name clash
                return scan_digits(f_it, f_end, std::distance(f_it, f_end));
            } else {
                return true;
            }
        } else {
            return false;
        }
    }

    static bool scan_digits(std::string::const_iterator& it, std::string::const_iterator end, int n) {
        for (; n > 0; --n) {
            if (it == end) {
                return false;
            }

            char c = *it++;
//            std::cout << "[" << c << "]" << std::boolalpha << (it == end) << std::endl;
            if (!std::isdigit(c)) {
                return false;
            }
        }

        return true;
    }

    template<typename T>
    static int digits(T number) {
        int digits = 0;
        if (number < 0) {
            digits = 1;
        }

        while (number) {
            number /= 10;
            digits++;
        }

        return digits;
    }

    std::vector<std::string> sort(const std::vector<std::string>& filenames) const {
        std::vector<std::string> result = filenames;
        std::sort(result.begin(), result.end(), [this](const std::string& lhs, const std::string& rhs) {
            return backend.changed(lhs) < backend.changed(rhs);
        });
        return result;
    }

    std::vector<std::pair<std::string, std::string> >
    cumilative(const std::vector<std::string>& filenames, const std::string& pattern, int backups) const {
        std::vector<std::pair<std::string, std::string>> result;
        int pos = pattern.find("%N");
        if (pos == -1) {
            return result;
        }

        std::cout << pos << std::endl;
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

        std::cout << pos << std::endl;
        int counter = 0;
        for (const std::string& filename : filenames) {
            std::cout << filename << std::endl;
            if (counter >= backups) {
                break;
            }

            std::string fn = filename;
            std::string current = std::string(fn.begin() + pos, fn.begin() + pos + 1);
            int i = atoi(current.data());
            i++;
            fn.replace(pos, 1, std::to_string(i));
            result.push_back(std::make_pair(filename, fn));
            counter++;
        }
        return result;
    }

    std::string format(const std::string& pattern) const {
        std::string filename = pattern;
        boost::algorithm::replace_all(filename, "%N", "1");
        std::ostringstream stream;
        std::time_t time = std::time(nullptr);
        stream << std::put_time(std::gmtime(&time), filename.data());
        return stream.str();
    }
};

} // namespace sink

} // namespace blackhole
