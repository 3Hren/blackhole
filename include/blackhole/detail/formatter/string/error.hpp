#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

class parser_error_t : public std::runtime_error {
    const std::size_t pos;
    const std::string inspect;

public:
    parser_error_t(std::size_t pos, const std::string& pattern, const std::string& reason);
    ~parser_error_t() noexcept;

    auto position() const noexcept -> std::size_t;
    auto detail() const noexcept -> const std::string&;
};

class broken_parser : public parser_error_t {
public:
    broken_parser(std::size_t pos, const std::string& pattern);
};

class illformed : public parser_error_t {
public:
    illformed(std::size_t pos, const std::string& pattern);
};

class invalid_placeholder : public parser_error_t {
public:
    invalid_placeholder(std::size_t pos, const std::string& pattern);
};

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
