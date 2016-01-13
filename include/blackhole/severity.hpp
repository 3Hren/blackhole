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

    constexpr auto operator==(const severity_t& other) const noexcept -> bool {
        return value == other.value;
    }
};

}  // namespace v1
}  // namespace blackhole
