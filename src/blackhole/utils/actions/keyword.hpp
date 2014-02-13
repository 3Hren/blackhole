#pragma once

namespace blackhole {

namespace action {

struct LessEq {
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

} // namespace action

} // namespace blackhole
