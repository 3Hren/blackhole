#pragma once

namespace blackhole {

namespace helper {

struct LessEqThan {
    template<typename L, typename R>
    static bool execute(const L& left, const R& right) {
        return left <= right;
    }
};

struct Eq {
    template<typename L, typename R>
    static bool execute(const L& left, const R& right) {
        return left == right;
    }
};

} // namespace helper

} // namespace blackhole
