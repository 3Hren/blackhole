#pragma once

#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "blackhole/error.hpp"
#include "blackhole/detail/stream/stream.hpp"
#include "blackhole/formatter/base.hpp"
#include "blackhole/formatter/map/value.hpp"
#include "blackhole/formatter/string/builder.hpp"
#include "blackhole/formatter/string/config.hpp"
#include "blackhole/record.hpp"
#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

namespace formatter {

class string_t : public base_t {
    const std::string pattern;
    const std::vector<string::builder::type> formatters;

public:
    typedef string::config_t config_type;

    static const char* name() {
        return "string";
    }

    string_t(const std::string& pattern) :
        pattern(pattern),
        formatters(string::formatter_builder_t::build(pattern))
    {}

    string_t(const config_type& config) :
        pattern(config.pattern),
        formatters(string::formatter_builder_t::build(config.pattern))
    {}

    std::string format(const log::record_t& record) const {
        std::string buffer;
        blackhole::aux::attachable_ostringstream stream;
        stream.attach(buffer);
        try {
            for (auto it = formatters.begin(); it != formatters.end(); ++it) {
                const string::builder::type& formatter = *it;
                formatter(stream, mapper, record.attributes);
                stream.flush();
            }
            return buffer;
        } catch (const error_t& err) {
            throw error_t("bad format string '%s': %s", pattern, err.what());
        }
    }
};

} // namespace formatter

template<>
struct factory_traits<formatter::string_t> {
    typedef formatter::string_t::config_type config_type;

    static void map_config(const aux::extractor<formatter::string_t>& ex, config_type& cfg) {
        cfg.pattern = ex["pattern"].get<std::string>();
    }
};

} // namespace blackhole
