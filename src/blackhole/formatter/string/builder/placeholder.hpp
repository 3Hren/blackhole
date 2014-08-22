#pragma once

#include <sstream>
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

struct placeholder_t {
    const std::string placeholder;

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t& mapper,
                    const attribute::set_view_t& attributes) const
    {
        if (auto attribute = attributes.find(placeholder)) {
            mapper(stream, placeholder, attribute->value);
            return;
        }

        throw error_t("key '%s' was not provided", placeholder);
    }
};

struct optional_placeholder_t {
    std::string placeholder;
    std::string prefix;
    std::string suffix;

    optional_placeholder_t(const std::string& placeholder) {
        std::string token;
        for (auto it = placeholder.begin(); it != placeholder.end(); ++it) {
            if (*it == '[' && it == placeholder.begin()) {
                continue;
            }

            if (*it == '[' && it != placeholder.begin() && *(it - 1) != '\\') {
                boost::algorithm::replace_all(token, "\\[", "[");
                boost::algorithm::replace_all(token, "\\]", "]");
                this->prefix = token;
                token.clear();
                continue;
            } else if (*it == ']' && it != placeholder.begin() && *(it - 1) != '\\') {
                boost::algorithm::replace_all(token, "\\[", "[");
                boost::algorithm::replace_all(token, "\\]", "]");
                this->placeholder = token;
                token.clear();
                continue;
            }

            token.push_back(*it);
        }

        boost::algorithm::replace_all(token, "\\[", "[");
        boost::algorithm::replace_all(token, "\\]", "]");
        this->suffix = token;
    }

    void operator()(blackhole::aux::attachable_ostringstream& stream,
                    const mapping::value_t& mapper,
                    const attribute::set_view_t& attributes) const
    {
        if (auto attribute = attributes.find(placeholder)) {
            stream.rdbuf()->storage()->append(prefix);
            mapper(stream, placeholder, attribute->value);
            stream.rdbuf()->storage()->append(suffix);
        }
    }
};

} // namespace builder

} // namespace string

} // namespace formatter

} // namespace blackhole
