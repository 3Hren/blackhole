#pragma once

#include <functional>
#include <iosfwd>

#include "blackhole/sink.hpp"
#include "blackhole/sink/console.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {

class console_t : public sink_t {
    std::ostream& stream_;
    std::unique_ptr<filter_t> filter;
    std::function<termcolor_t(const record_t& record)> mapping_;

public:
    console_t();
    console_t(std::unique_ptr<filter_t> filter);
    console_t(std::ostream& stream, std::function<termcolor_t(const record_t& record)> mapping);

    auto stream() noexcept -> std::ostream&;
    auto mapping(const record_t& record) const -> termcolor_t;

    auto emit(const record_t& record, const string_view& formatted) -> void override;
};

}  // namespace sink
}  // namespace v1
}  // namespace blackhole
