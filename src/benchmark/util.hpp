#pragma once

#include <memory>
#include <type_traits>

#include <blackhole/frontend.hpp>

template<class L, class F, class S>
struct initializer_t {
    std::unique_ptr<F> f;

    initializer_t(std::unique_ptr<F> f) :
        f(std::move(f))
    {}

    initializer_t(initializer_t&& other) :
        f(std::move(other.f))
    {}

    template<class... Args>
    L operator()(Args&&... args) {
        std::unique_ptr<S> s(new S(std::forward<Args>(args)...));
        std::unique_ptr<blackhole::frontend_t<F, S>> frontend(
            new blackhole::frontend_t<F, S>(std::move(f), std::move(s))
        );

        L log;
        log.add_frontend(std::move(frontend));
        return log;
    }
};

template<class L, class F, class S, class... Args>
inline
initializer_t<L, F, S>
initialize(Args&&... args) {
    std::unique_ptr<F> formatter(new F(std::forward<Args>(args)...));
    return initializer_t<L, F, S>(std::move(formatter));
}
