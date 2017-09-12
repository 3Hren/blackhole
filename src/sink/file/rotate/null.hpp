#pragma once

#include "../../../memory.hpp"
#include "../rotate.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace rotate {

class null_rotate_t : public rotate_t {
public:
    auto should_rotate() -> bool override {
        return false;
    }
};

class null_factory_t : public rotate_factory_t {
public:
    auto create(const std::string&) const -> std::unique_ptr<rotate_t> override {
        return blackhole::make_unique<null_rotate_t>();
    }
};

} // namespace rotate
} // namespace file
} // namespace sink
} // namespace v1
} // namespace blackhole
