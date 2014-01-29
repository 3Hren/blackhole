#pragma once

#include <cstdint>

#include "blackhole/sink/files/rotation/naming/comparator.hpp"
#include "blackhole/sink/files/rotation/naming/helpers.hpp"

namespace blackhole {

namespace sink {

namespace matching {

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

private:
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
                    if (!all_digits(f_it, f_end, 2)) {
                        return false;
                    }
                    ++p_it;
                    break;
                case 'Y':
                    if (!all_digits(f_it, f_end, 4)) {
                        return false;
                    }
                    ++p_it;
                    break;
                case 'N':
                    if (!all_digits(f_it, f_end, digits(backups))) {
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
                return all_digits(f_it, f_end, std::distance(f_it, f_end));
            } else {
                return true;
            }
        } else {
            return false;
        }
    }
};

namespace counting {

//! Locate real counter position in any matched filename using given pattern.
inline int pos(const std::string& pattern) {
    int pos = pattern.find("%N");
    if (pos == -1) {
        return -1;
    }

    bool placeholder_expected = false;
    for (auto it = pattern.begin(); it != pattern.end() && it != pattern.begin() + pos; ++it) {
        const char c = *it;

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

    return pos;
}

} // namespace counting

} // namespace matching

} // namespace sink

} // namespace blackhole
