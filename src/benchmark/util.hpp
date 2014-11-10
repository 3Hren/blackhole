#pragma once

#include <memory>
#include <type_traits>

#include <blackhole/frontend.hpp>

namespace initializer {

template<class L>
struct mod_t {
    L log;

    template<class... Args>
    L operator()(std::function<void(L&)> fn = &mod_t::nope) {
        fn(log);
        return std::move(log);
    }

    static inline void nope(L&) {}
};

template<class L, class F, class S>
struct log_t {
    std::unique_ptr<F> f;
    std::unique_ptr<S> s;

    template<class... Args>
    mod_t<L>
    operator()(Args&&... args) {
        std::unique_ptr<blackhole::frontend_t<F, S>> frontend(
            new blackhole::frontend_t<F, S>(std::move(f), std::move(s))
        );

        L log(std::forward<Args>(args)...);
        log.add_frontend(std::move(frontend));

        return mod_t<L> { std::move(log) };
    }
};

template<class L, class F, class S>
struct sink_t {
    std::unique_ptr<F> f;

    template<class... Args>
    log_t<L, F, S>
    operator()(Args&&... args) {
        return log_t<L, F, S> { std::move(f), std::unique_ptr<S>(new S(std::forward<Args>(args)...)) };
    }
};

} // namespace initializer

template<class L, class F, class S, class... Args>
inline
initializer::sink_t<L, F, S>
initialize(Args&&... args) {
    return initializer::sink_t<L, F, S> { std::unique_ptr<F>(new F(std::forward<Args>(args)...)) };
}

namespace filter_by {

template<typename Level>
void verbosity(blackhole::verbose_logger_t<Level>& log, Level level) {
    log.verbosity(level);
}

template<typename Level>
void verbosity_filter(blackhole::verbose_logger_t<Level>& log, Level level) {
    log.set_filter(blackhole::keyword::severity<Level>() >= level);
}

} // namespace filter_by
