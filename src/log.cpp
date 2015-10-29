#include "blackhole/log.hpp"

namespace blackhole {

struct log_t::inner_t {};

log_t::log_t():
    inner(new inner_t)
{}

log_t::~log_t() {}

void
log_t::info(string_view message) {
}

void
log_t::info(string_view message, const attributes_t& attributes) {
    for (const auto& attribute : attributes) {
    }
}

}  // namespace blackhole
