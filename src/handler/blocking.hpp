#pragma once

#include <memory>
#include <vector>

#include "blackhole/handler.hpp"
#include "blackhole/forward.hpp"

namespace blackhole {
inline namespace v1 {
namespace handler {

class blocking_t : public handler_t {
    std::unique_ptr<formatter_t> formatter;
    std::vector<std::unique_ptr<sink_t>> sinks;

public:
    blocking_t(std::unique_ptr<formatter_t> formatter, std::vector<std::unique_ptr<sink_t>> sinks);

    virtual auto handle(const record_t& record) -> void override;
};

}  // namespace handler
}  // namespace v1
}  // namespace blackhole
