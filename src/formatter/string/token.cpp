#include "blackhole/detail/formatter/string/token.hpp"

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {
namespace placeholder {

generic_t::generic_t(std::string name) noexcept :
    name(std::move(name)),
    spec("{}")
{}

generic_t::generic_t(std::string name, std::string spec) noexcept :
    name(std::move(name)),
    spec(std::move(spec))
{}

message_t::message_t() noexcept : spec("{}") {}
message_t::message_t(std::string spec) noexcept : spec(std::move(spec)) {}

template<typename T>
severity<T>::severity() noexcept : spec("{}") {}

template<typename T>
severity<T>::severity(std::string spec) noexcept : spec(std::move(spec)) {}

timestamp<num>::timestamp() noexcept : spec("{}") {}
timestamp<num>::timestamp(std::string spec) noexcept : spec(std::move(spec)) {}

timestamp<user>::timestamp() noexcept :
    pattern(),
    spec("{}")
{}

timestamp<user>::timestamp(std::string pattern, std::string spec) noexcept :
    pattern(std::move(pattern)),
    spec(std::move(spec))
{}

template<typename T>
process<T>::process() noexcept : spec("{}") {}

template<typename T>
process<T>::process(std::string spec) noexcept : spec(std::move(spec)) {}

template struct severity<num>;
template struct severity<user>;

template struct process<id>;
template struct process<name>;

}  // namespace placeholder
}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
