#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace formatter {
namespace string {

class parser_error_t : public std::runtime_error {
    const std::size_t pos;
    const std::string inspect;

public:
    parser_error_t(std::size_t pos, const std::string& pattern, const std::string& reason);
    parser_error_t(const parser_error_t& other) = default;

    ~parser_error_t() noexcept;

    auto position() const noexcept -> std::size_t;
    auto detail() const noexcept -> const std::string&;
};

class broken_t : public parser_error_t {
public:
    broken_t(std::size_t pos, const std::string& pattern);
};

class illformed_t : public parser_error_t {
public:
    illformed_t(std::size_t pos, const std::string& pattern);
};

class invalid_placeholder_t : public parser_error_t {
public:
    invalid_placeholder_t(std::size_t pos, const std::string& pattern);
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
