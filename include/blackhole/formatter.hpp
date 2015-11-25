#pragma once

namespace blackhole {

class record_t;

class writer_t;

class formatter_t {
public:
    formatter_t() = default;
    formatter_t(const formatter_t& other) = default;

    virtual ~formatter_t() {}

    virtual auto format(const record_t& record, writer_t& writer) -> void = 0;
};

}  // namespace blackhole
