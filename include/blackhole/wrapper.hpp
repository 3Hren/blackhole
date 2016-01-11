#pragma once

#include "blackhole/logger.hpp"

#include "blackhole/attribute.hpp"

namespace blackhole {
inline namespace v1 {

class wrapper_t : public logger_t {
    logger_t& inner;
    attributes_t storage;
    view_of<attributes_t>::type attributes_view;

public:
    wrapper_t(logger_t& log, attributes_t attributes);

    auto attributes() const noexcept -> const view_of<attributes_t>::type& {
        return attributes_view;
    }

    // TODO: auto filter(filter_t fn) -> void;

    auto log(int severity, const string_view& message) -> void;
    auto log(int severity, const string_view& message, attribute_pack& pack) -> void;
    auto log(int severity, const string_view& pattern, attribute_pack& pack, const supplier_t& supplier) -> void;

    /// Returns a scoped attributes context by delegating invocation to the underlying logger type.
    auto context() -> boost::thread_specific_ptr<scoped_t>*;
};

}  // namespace v1
}  // namespace blackhole
