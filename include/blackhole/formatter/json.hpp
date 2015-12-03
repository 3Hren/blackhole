#pragma once

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

class json_t : public formatter_t {
public:
    auto format(const record_t& record, writer_t& writer) -> void;
};

}  // namespace formatter
}  // namespace blackhole
