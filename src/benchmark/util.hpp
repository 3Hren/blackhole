#pragma once

#include <memory>
#include <type_traits>

#include <blackhole/frontend.hpp>

template<class L, class F, class S>
struct initializer_t {
    std::unique_ptr<F> f;
    std::unique_ptr<S> s;
    std::unique_ptr<L> l;

    // TODO: GCC 4.4. Hack.
    initializer_t() = default;
    initializer_t(initializer_t&& other) :
        f(std::move(other.f)),
        s(std::move(other.s)),
        l(std::move(other.l))
    {}

    template<class... Args>
    initializer_t& formatter(Args&&... args) {
        f = std::unique_ptr<F>(new F(std::forward<Args>(args)...));
        return *this;
    }

    template<class... Args>
    initializer_t& sink(Args&&... args) {
        s = std::unique_ptr<S>(new S(std::forward<Args>(args)...));
        return *this;
    }

    template<class... Args>
    initializer_t& log(Args&&... args) {
        BOOST_ASSERT(f);
        BOOST_ASSERT(s);

        std::unique_ptr<blackhole::frontend_t<F, S>> frontend(
            new blackhole::frontend_t<F, S>(std::move(f), std::move(s))
        );

        l = std::unique_ptr<L>(new L(std::forward<Args>(args)...));
        l->add_frontend(std::move(frontend));
        return *this;
    }

    template<class... Args>
    initializer_t& mod(std::function<void(L&)> fn = &initializer_t::nope) {
        BOOST_ASSERT(l);
        fn(*l);
        return *this;
    }

    L get() {
        BOOST_ASSERT(l);

        return std::move(*l);
    }

private:
    static inline void nope(L&) {}
};

template<class L, class F, class S>
inline
initializer_t<L, F, S>
initialize() {
    return initializer_t<L, F, S>();
}

namespace filter_by {

template<typename Level>
void verbosity(blackhole::verbose_logger_t<Level>& log, Level level) {
    log.set_filter(level);
}

} // namespace filter_by
