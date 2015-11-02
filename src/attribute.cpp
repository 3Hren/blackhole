#include "blackhole/attribute.hpp"

namespace blackhole {
namespace attribute {

value_t::value_t(const owned_t& val):
    inner(boost::apply_visitor(from_owned_t(), val.inner))
{}

}  // namespace attribute
}  // namespace blackhole
