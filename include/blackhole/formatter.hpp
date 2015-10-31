#pragma once

namespace blackhole {

class record_t;

class writer_t;

class formatter_t {
public:
    virtual ~formatter_t() {}

    virtual auto format(const record_t& record, writer_t& writer) -> void = 0;
};

}  // namespace blackhole
