#pragma once

#include <functional>

#include "blackhole/attribute/set.hpp"
#include "blackhole/detail/config/inline.hpp"
#include "blackhole/forwards.hpp"

namespace blackhole {

namespace policy {

template<class T, typename... Args>
class filter_t {
    typedef T polulator_type;

public:
    typedef std::function<bool(const attribute::combined_view_t&, const Args&...)> function_type;

public:
    BLACKHOLE_ALWAYS_INLINE
    static inline
    bool
    filter(const function_type& fn, const attribute::combined_view_t& view, const Args&... args) {
        return fn(view, args...);
    }

    BLACKHOLE_ALWAYS_INLINE
    static inline
    void populate_with(attribute::set_t& internal, const Args&... args) {
        polulator_type::insert(internal, args...);
    }
};

template<>
class filter_t<void> {
public:
    typedef std::function<bool(const attribute::combined_view_t&)> function_type;

public:
    BLACKHOLE_ALWAYS_INLINE
    static inline
    bool
    filter(const function_type& fn, const attribute::combined_view_t& view) {
        return fn(view);
    }

    BLACKHOLE_ALWAYS_INLINE
    static inline
    void populate_with(attribute::set_t&) {}
};

} // namespace policy

} // namespace blackhole
