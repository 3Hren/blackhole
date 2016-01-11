#include "blackhole/detail/formatter/string/error.hpp"

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {

parser_error_t::parser_error_t(std::size_t pos, const std::string& pattern, const std::string& reason) :
    std::runtime_error("parser error: " + reason),
    pos(pos),
    inspect("parser error: " +  reason + "\n" +
        pattern + "\n" +
        std::string(pos, '~') + "^"
    )
{}

parser_error_t::~parser_error_t() noexcept {}

auto
parser_error_t::position() const noexcept -> std::size_t {
    return pos;
}

auto
parser_error_t::detail() const noexcept -> const std::string& {
    return inspect;
}

broken_t::broken_t(std::size_t pos, const std::string& pattern) :
    parser_error_t(pos, pattern, "broken parser")
{}

illformed_t::illformed_t(std::size_t pos, const std::string& pattern) :
    parser_error_t(pos, pattern, "illformed pattern")
{}

invalid_placeholder_t::invalid_placeholder_t(std::size_t pos, const std::string& pattern) :
    parser_error_t(pos, pattern, "invalid placeholder name (only [a-zA-Z0-9_] allowed)")
{}

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
