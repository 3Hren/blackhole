#pragma once

#include "blackhole/formatter.hpp"

namespace blackhole {
namespace formatter {

class string_t : formatter_t {
public:
    explicit string_t(std::string pattern) {}

    auto format(const record_t& record, writer_t& writer) -> void {}
};

}  // namespace formatter
}  // namespace blackhole
