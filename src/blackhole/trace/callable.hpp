#pragma once

#include <type_traits>

#include "context.hpp"
#include "random.hpp"
#include "span.hpp"

namespace trace {

template<typename F>
class callable_t {
public:
    typedef F function_type;

private:
    const span_t span;
    function_type f;

public:
    callable_t(function_type f) :
        span(this_thread::current_span()),
        f(std::move(f))
    {}

    template<typename... Args>
    void operator()(Args&&... args) {
        trace::context_t<> context(span);
        f(std::forward<Args>(args)...);
    }
};

template<typename F>
callable_t<F> wrap(F&& f) {
    return callable_t<F>(std::forward<F>(f));
}

} // namespace trace
