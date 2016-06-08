#pragma once

#include <functional>
#include <iosfwd>

#include "blackhole/sink.hpp"
#include "blackhole/sink/console.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

typedef std::function<termcolor_t(const record_t& record)> termcolor_map_t;

class console_t : public sink_t {
    std::ostream& stream_;
    termcolor_map_t colormap;

public:
    console_t();

    /// Constructs a new console sink, which will write all incoming events to the specified
    /// stream using the given terminal color mapping to colorize the output.
    ///
    /// Useful for testing reasons for example.
    console_t(std::ostream& stream, termcolor_map_t colormap);

    auto stream() noexcept -> std::ostream&;

    /// Writes the formatted message into the attached output stream.
    ///
    /// Note that the message may be anticipatorily colored using severity information from the
    /// associated record.
    auto emit(const record_t& record, const string_view& formatted) -> void override;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
