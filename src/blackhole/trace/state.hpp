#pragma once

#include <boost/thread.hpp>

#include "blackhole/utils/nullptr.hpp"

#include "forwards.hpp"

namespace trace {

class state_t {
    boost::thread_specific_ptr<span_t> span;

    template<class Random> friend class context_t;
    friend const span_t& this_thread::current_span();

public:
    static state_t& instance() {
        static state_t self;
        return self;
    }

private:
    state_t() :
        span(&state_t::deleter)
    {}

    span_t* get() const {
        return span.get();
    }

    void reset(span_t* span = nullptr) {
        this->span.reset(span);
    }

    static void deleter(span_t*) {}
};

} // namespace trace
