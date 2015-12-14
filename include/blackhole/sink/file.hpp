#pragma once

#include "blackhole/sink.hpp"

namespace blackhole {

class config_t;

template<typename>
struct factory;

}  // namespace blackhole

namespace blackhole {
namespace sink {

class file_t : public sink_t {
    class inner_t;
    class properties_t;

    std::unique_ptr<inner_t> inner;

public:
    class builder_t;

    /// Constructs a file sink, which will write all incoming events to the file or files located at
    /// the specified path.
    ///
    /// No real file will be opened at construction time.
    explicit file_t(const std::string& path);

    ~file_t();

    auto filter(const record_t& record) -> bool;
    auto execute(const record_t& record, const string_view& formatted) -> void;

private:
    file_t(std::unique_ptr<inner_t> inner);
    file_t(std::unique_ptr<properties_t> properties);
};

class file_t::builder_t {
    std::unique_ptr<file_t::properties_t> properties;

public:
    explicit builder_t(const std::string& path);

    auto interval(std::size_t count) -> builder_t&;
    auto build() -> file_t;
};

}  // namespace sink

template<>
struct factory<sink::file_t> {
    static auto type() -> const char*;
    static auto from(const config_t& config) -> sink::file_t;
};

}  // namespace blackhole
