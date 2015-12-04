#pragma once

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

// TODO: Add severity mapping support.
// TODO: Add timestamp mapping support.
class json_t : public formatter_t {
public:
    auto format(const record_t& record, writer_t& writer) -> void;
};

}  // namespace formatter
}  // namespace blackhole
