#pragma once

#include <functional>

#include "attribute.hpp"

namespace blackhole {

typedef std::function<bool(const attribute_set_view_t& attributes)> filter_t;

struct default_filter_t {
    static default_filter_t& instance() {
        static default_filter_t filter;
        return filter;
    }

    bool operator()(const attribute_set_view_t&) {
        return true;
    }

private:
    default_filter_t() {}
};

} // namespace blackhole
