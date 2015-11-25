#pragma once

#include <memory>

namespace blackhole {

class formatter_t;
class record_t;
class sink_t;

class handler_t {
public:
    virtual ~handler_t() {}

    /// \warning must be thread-safe.
    virtual auto execute(const record_t& record) -> void = 0;

    virtual auto set(std::unique_ptr<formatter_t> formatter) -> void = 0;
    virtual auto add(std::unique_ptr<sink_t> sink) -> void = 0;
};

}  // namespace blackhole
