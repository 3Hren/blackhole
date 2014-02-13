#pragma once

#include <iostream>
#include <ostream>
#include <string>

#include <boost/assert.hpp>

#include "blackhole/repository/factory/traits.hpp"

namespace blackhole {

namespace sink {

namespace stream {

struct config_t {
    std::string output;
};

} // namespace stream

class stream_t{
    std::ostream& stream;
public:
    enum class output_t {
        stdout,
        stderr
    };

    typedef stream::config_t config_type;

    static const char* name() {
        return "stream";
    }

    stream_t(output_t output) :
        stream(stream_factory_t::get(output))
    {}

    stream_t(const std::string& output) :
        stream(stream_factory_t::get(output))
    {}

    stream_t(const config_type& config) :
        stream(stream_factory_t::get(config.output))
    {}

    void consume(const std::string& message) {
        stream << message << std::endl;
    }

private:
    struct stream_factory_t {
        static std::ostream& get(output_t output) {
            switch (output) {
            case output_t::stdout:
                return std::cout;
            case output_t::stderr:
                return std::cerr;
            default:
                BOOST_ASSERT(false);
            }
        }

        static std::ostream& get(const std::string& output) {
            output_t out = output_t::stdout;

            if (output == "stdout") {
                out = output_t::stdout;
            } else if (output == "stderr") {
                out = output_t::stderr;
            }

            return get(out);
        }
    };
};

} // namespace sink

template<>
struct factory_traits<sink::stream_t> {
    typedef sink::stream_t sink_type;
    typedef sink_type::config_type config_type;

    static void map_config(const aux::extractor<sink_type>& ex, config_type& config) {
        ex["output"].to(config.output);
    }
};

} // namespace blackhole
