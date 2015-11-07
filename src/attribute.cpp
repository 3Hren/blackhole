#include "blackhole/attribute.hpp"

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

namespace blackhole {
namespace attribute {
namespace {

struct from_owned_t: public boost::static_visitor<value_t::type> {
    template<typename T>
    auto operator()(const T& val) const -> value_t::type {
        return val;
    }
};

}  // namespace

value_t::value_t(const owned_t& val):
    inner(boost::apply_visitor(from_owned_t(), val.inner))
{}

}  // namespace attribute
}  // namespace blackhole
