#pragma once

namespace blackhole {
inline namespace v1 {

class record_t;

class writer_t;

class formatter_t {
public:
    formatter_t() = default;
    formatter_t(const formatter_t& other) = default;
    formatter_t(formatter_t&& other) = default;

    virtual ~formatter_t() {}

    virtual auto format(const record_t& record, writer_t& writer) -> void = 0;
};

}  // namespace v1
}  // namespace blackhole
