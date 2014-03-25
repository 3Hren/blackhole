#pragma once

#include <cstdint>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "blackhole/sink/files/rotation/naming/basename.hpp"
#include "blackhole/sink/files/rotation/naming/comparator.hpp"
#include "blackhole/sink/files/rotation/naming/helpers.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

struct counter_t {
    const std::string prefix;
    const std::string suffix;
    const uint width;

    counter_t(const std::string& prefix, const std::string& suffix, uint width) :
        prefix(prefix),
        suffix(suffix),
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

    bool valid() const {
        return width != 0;
    }

    //! I just leave these examples as mini documentation.
    /*!
     * pattern:     test.log.%Y%m%d.%N      %(filename)s.%N     %(filename)s.%Y%m%d.%N
     * basename:    test.log.%Y%m%d.%N      test.log.%N         test.log.%Y%m%d.%N
     * filename:    test.log.20140130.1     test.log.1          test.log.20140130.1
     * next:        test.log.20140130.2     test.log.2          test.log.20140130.2
     */
    std::string next(const std::string& filename, uint value) const {
        BOOST_ASSERT(static_cast<int>(filename.size()) -
                     static_cast<int>(prefix.size()) -
                     static_cast<int>(suffix.size()) >= 0);

        std::ostringstream stream;
        stream << std::string(filename.begin(), filename.begin() + prefix.size())
               << std::setfill('0') << std::setw(width) << (value + 1)
               << std::string(filename.begin() + prefix.size() + std::max(width, naming::digits(value)),
                              filename.end());
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

        const auto& placeholders = available_placeholders();
        for (auto it = placeholders.begin(); it != placeholders.end(); ++it) {
            boost::replace_all(prefix, it->first, it->second);
            boost::replace_all(suffix, it->first, it->second);
        }

        return counter_t(std::move(prefix), std::move(suffix), width);
    }

private:
    static uint cast(const std::string& str, uint default_value = 0) {
        try {
            return boost::lexical_cast<uint>(str);
        } catch (const boost::bad_lexical_cast&) {
            return default_value;
        }
    }

    static const std::map<std::string, std::string>& available_placeholders() {
        static std::map<std::string, std::string> PLACEHOLDERS = {
            { "%Y", "YYYY" },
            { "%m", "mm" },
            { "%d", "dd" },
            { "%H", "HH" },
            { "%M", "MM" },
            { "%s", "ss" }
        };
        return PLACEHOLDERS;
    }
};

} // namespace rotation

} // namespace sink

} // namespace blackhole
