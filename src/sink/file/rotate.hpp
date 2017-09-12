#pragma once

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {

class rotate_t {
public:
    virtual ~rotate_t() = default;
    virtual auto should_rotate() -> bool = 0;
};

class rotate_factory_t {
public:
    virtual ~rotate_factory_t() = default;
    virtual auto create(const std::string& filename) const -> std::unique_ptr<rotate_t> = 0;
};

} // namespace file
} // namespace sink
} // namespace v1
} // namespace blackhole
