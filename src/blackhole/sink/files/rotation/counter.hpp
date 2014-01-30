#pragma once

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "blackhole/sink/files/rotation/naming.hpp"

namespace blackhole {

namespace sink {

namespace rotation {

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
               << std::string(filename.begin() + prefix.size() + std::max(width, matching::digits(value)), filename.end());
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

} // namespace rotation

} // namespace sink

} // namespace blackhole
