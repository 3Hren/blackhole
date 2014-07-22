#pragma once

#include <type_traits>

#include "span.hpp"
#include "state.hpp"

namespace trace {

template<class Random>
class context_t {
public:
    typedef Random random_type;
    typedef typename random_type::value_type value_type;

private:
    span_t span;
    span_t* parent;

public:
    context_t() :
        span(generate()),
        parent(state_t::instance().get())
    {
        state_t::instance().reset(&this->span);
    }

    context_t(value_type trace) :
        span(trace == 0 ? generate() : span_t(trace, trace)),
        parent(state_t::instance().get())
    {
        state_t::instance().reset(&this->span);
    }

    context_t(span_t span) :
        span(std::move(span)),
        parent(state_t::instance().get())
    {
        state_t::instance().reset(&this->span);
    }

    ~context_t() {
        state_t::instance().reset(parent);
    }

private:
    static span_t generate() {
        const value_type id = random_type::instance().next();
        if (auto current = state_t::instance().get()) {
            return span_t(current->trace, id, current->span);
        }

        return span_t(id, id);
    }
};

} // namespace trace

namespace this_thread {

inline const span_t& current_span() {
    auto span = trace::state_t::instance().get();
    return span ? *span : span_t::invalid();
}

} // namespace this_thread
