#pragma once

namespace blackhole {
inline namespace v1 {

template<typename T>
auto deleter_t::operator()(T* value) -> void {
    delete value;
}

}  // namespace v1
}  // namespace blackhole
