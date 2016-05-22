#pragma once

#include <functional>
#include <iosfwd>

#include "blackhole/sink.hpp"
#include "blackhole/sink/console.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

using experimental::color_t;

class console_t : public sink_t {
    std::ostream& stream;
    std::function<auto(const record_t& record) -> color_t> colormap;

public:
    console_t();
    console_t(std::ostream& stream, std::function<auto(const record_t& record) -> color_t> colormap);

    auto emit(const record_t& record, const string_view& formatted) -> void override;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
