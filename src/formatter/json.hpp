#pragma once

#include "blackhole/formatter.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {

class json_t : public formatter_t {
public:
    class properties_t;

private:
    class inner_t;

    std::unique_ptr<inner_t, deleter_t> inner;

public:
    /// Constructs a defaultly configured JSON formatter, which will produce plain trees with no
    /// filtering without adding a separator character at the end.
    json_t();
    json_t(properties_t properties);

    /// Returns true if there will be newline sequence added after each formatted message.
    auto newline() const noexcept -> bool;

    /// Returns true if the filtering policy is enabled.
    auto unique() const noexcept -> bool;

    /// Formats the given record by constructing a JSON tree with further serializing into the
    /// specified writer.
    ///
    /// \remark this method is thread safe.
    auto format(const record_t& record, writer_t& writer) -> void;
};

}  // namespace formatter
}  // namespace v1
}  // namespace blackhole
