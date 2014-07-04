#pragma once

#include <cstdint>
#include <ostream>

#include <boost/assert.hpp>

struct span_t {
    const std::uint64_t trace;
    const std::uint64_t span;
    const std::uint64_t parent;

    span_t(std::uint64_t trace, std::uint64_t span, std::uint64_t parent = 0) :
        trace(trace),
        span(span),
        parent(parent)
    {
        // There is special invalid state, that shouldn't be created manually.
        BOOST_ASSERT(trace != 0 && span != 0);
    }

    bool valid() const {
        return trace != 0 && span != 0;
    }

    static const span_t& invalid() {
        static const span_t span;
        return span;
    }

    bool operator==(const span_t& other) const {
        return trace == other.trace && span == other.span && parent == other.parent;
    }

    friend
    std::ostream&
    operator<<(std::ostream& stream, const span_t& span) {
        stream << "span("
               << span.trace << ", " << span.span << ", " << span.parent
               << ")";
        return stream;
    }

private:
    span_t() :
        trace(0),
        span(0),
        parent(0)
    {}
};
