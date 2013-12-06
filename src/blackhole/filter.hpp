#pragma once

#include <functional>

#include "attribute.hpp"

namespace blackhole {

typedef std::function<bool(const log::attributes_t& attributes)> filter_t;

struct default_filter_t {
    static default_filter_t& instance() {
        static default_filter_t filter;
        return filter;
    }

    bool operator()(const log::attributes_t&) {
        return true;
    }

private:
    default_filter_t() {}
};

} // namespace blackhole
