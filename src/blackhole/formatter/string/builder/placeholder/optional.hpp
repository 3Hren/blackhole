#pragma once

#include <string>

#include <boost/algorithm/string.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/error.hpp"
#include "blackhole/formatter/map/value.hpp"

namespace blackhole {

namespace formatter {

namespace string {

namespace builder {

namespace placeholder {

struct optional_t {
    std::string name;
    std::string prefix;
    std::string suffix;

    optional_t(const std::string& placeholder) {
        std::string token;
        for (auto it = placeholder.begin(); it != placeholder.end(); ++it) {
            if (*it == '[' && it == placeholder.begin()) {
                continue;
            }

            if (*it == '[' && it != placeholder.begin() && *(it - 1) != '\\') {
                boost::algorithm::replace_all(token, "\\[", "[");
                boost::algorithm::replace_all(token, "\\]", "]");
                prefix = token;
                token.clear();
                continue;
            } else if (*it == ']' && it != placeholder.begin() && *(it - 1) != '\\') {
                boost::algorithm::replace_all(token, "\\[", "[");
                boost::algorithm::replace_all(token, "\\]", "]");
                name = token;
                token.clear();
                continue;
            }

            token.push_back(*it);
        }

        boost::algorithm::replace_all(token, "\\[", "[");
        boost::algorithm::replace_all(token, "\\]", "]");
        suffix = token;
    }

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t& mapper,
                    const attribute::set_view_t& attributes) const
    {
        if (auto attribute = attributes.find(name)) {
            stream.rdbuf()->storage()->append(prefix);
            mapper(stream, name, attribute->value);
            stream.rdbuf()->storage()->append(suffix);
        }
    }
};

} // namespace placeholder

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
