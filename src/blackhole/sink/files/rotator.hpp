#pragma once

#include <cstdint>
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
        return false;
    }

    void rotate() const {
        const std::string& filename = backend.filename();

        backend.flush();
        backend.close();
        //!@todo: Implement rotation naming strategy, because N and DateTime naming are mutual exclusive.
        if (config.pattern.find("%N") != std::string::npos) {
            std::string suffix = config.pattern;
            boost::algorithm::replace_all(suffix, "%N", "%s");
            for (std::uint16_t i = config.backups - 1; i > 0; --i) {
                std::string oldname = filename + utils::format(suffix, i);
                if (backend.exists(oldname)) {
                    std::string newname = filename + utils::format(suffix, i + 1);
                    backend.rename(oldname, newname);
                }
            }

            if (backend.exists(filename)) {
                backend.rename(filename, filename + utils::format(suffix, 1));
            }
        }

        backend.open();
    }
};

}

}
