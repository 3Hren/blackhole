#pragma once

#include "blackhole/record.hpp"

namespace blackhole {
inline namespace v1 {

struct record_t::inner_t {
    std::reference_wrapper<const string_view> message;
    std::reference_wrapper<const string_view> formatted;

    severity_t severity;
    time_point timestamp;

    std::thread::native_handle_type tid;
    char __pad[16];

    std::reference_wrapper<const attribute_pack> attributes;
};

}  // namespace v1
}  // namespace blackhole
