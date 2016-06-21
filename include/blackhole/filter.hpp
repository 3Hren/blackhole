#pragma once

#include <memory>

namespace blackhole {
inline namespace v1 {

class record_t;

/// Represents logging filter interface.
///
/// Filters allow log events to be evaluated to determine if or how they should be published.
class filter_t {
public:
    enum class action_t {
        /// Don't know what to do. Pass to other filters, if there are some.
        neutral,
        /// Accept record immediately.
        accept,
        /// Deny record immediately.
        deny
    };

public:
    virtual ~filter_t() = default;
    virtual auto filter(const record_t& record) -> action_t = 0;
};

}  // namespace v1
}  // namespace blackhole
