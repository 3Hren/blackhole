#pragma once

namespace blackhole {
inline namespace v1 {

class severity_t {
    int value;

public:
    constexpr severity_t(int value) noexcept :
        value(value)
    {}

    constexpr operator int() const noexcept {
        return value;
    }
};

}  // namespace v1
}  // namespace blackhole
