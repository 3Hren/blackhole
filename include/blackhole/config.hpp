#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace blackhole {
namespace config {

class bad_cast : public std::logic_error {
public:
    bad_cast() : std::logic_error("bad cast") {}
};

class bad_optional_access : public std::logic_error {
public:
    bad_optional_access() : std::logic_error("not engaged") {}
};

template<typename T>
class monadic;

}  // namespace config

class config_t {
public:
    typedef std::function<auto(const config_t&) -> void> each_function;
    typedef std::function<auto(const std::string&, const config_t&) -> void> member_function;

public:
    virtual ~config_t() {}

    virtual auto operator[](const std::size_t& idx) const -> config::monadic<config_t> = 0;
    virtual auto operator[](const std::string& key) const -> config::monadic<config_t> = 0;

    virtual auto to_bool() const -> bool = 0;
    virtual auto to_i64() const -> std::int64_t = 0;
    virtual auto to_u64() const -> std::uint64_t = 0;
    virtual auto to_double() const -> double = 0;
    virtual auto to_string() const -> std::string = 0;

    virtual auto each(const each_function& fn) -> void = 0;
    virtual auto each_map(const member_function& fn) -> void = 0;
};

using config::bad_cast;
using config::bad_optional_access;

}  // namespace blackhole
