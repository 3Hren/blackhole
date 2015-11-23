#pragma once

namespace blackhole {

class record_t;

class handler_t {
public:
    virtual ~handler_t() {}

    /// \warning must be thread-safe.
    virtual auto execute(const record_t& record) -> void = 0;
};

}  // namespace blackhole
