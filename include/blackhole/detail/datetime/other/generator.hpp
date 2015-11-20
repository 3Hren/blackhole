#pragma once

#include <string>
#include <vector>

namespace blackhole {
namespace detail {
namespace datetime {

struct context_t;
typedef void(*token_t)(context_t&);

class generator_t {
    std::vector<token_t> tokens;
    std::vector<std::string> literals;

public:
    generator_t(std::vector<token_t> tokens, std::vector<std::string> literals);

    template<typename Stream>
    auto operator()(Stream& stream, const std::tm& tm, std::uint64_t usec = 0) const -> void;
};

auto make_generator(const std::string& pattern) -> generator_t;

}  // namespace datetime
}  // namespace detail
}  // namespace blackhole
