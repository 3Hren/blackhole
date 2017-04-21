#pragma once

#include <string>
#include <vector>

#include "blackhole/extensions/format.hpp"

namespace blackhole {
inline namespace v1 {
namespace datetime {

struct context_t;
typedef void (*token_t)(context_t&);
typedef fmt::MemoryWriter writer_type;

class generator_t {
    std::vector<token_t> tokens;
    std::vector<std::string> literals;

public:
    generator_t(std::vector<token_t> tokens, std::vector<std::string> literals);

    auto operator()(writer_type& stream, const std::tm& tm, std::uint64_t usec = 0) const -> void;
};

auto make_generator(const std::string& pattern) -> generator_t;

} // namespace datetime
} // namespace v1
} // namespace blackhole
