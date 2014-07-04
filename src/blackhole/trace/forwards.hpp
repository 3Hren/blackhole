#pragma once

#include <cstdint>

struct span_t;

template<typename Value>
struct distribution_t;

template<typename Distribution = distribution_t<std::uint64_t>>
class random_t;

namespace this_thread {

const span_t& current_span();

} // namespace this_thread

namespace trace {

template<class Random = random_t<>>
class context_t;

} // namespace trace
