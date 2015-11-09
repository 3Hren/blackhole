#include "blackhole/attribute.hpp"

#include <boost/variant/static_visitor.hpp>

namespace blackhole {
namespace attribute {
namespace {

struct into_view: public boost::static_visitor<view_t::type> {
    typedef view_t::type result_type;

    template<typename T>
    auto operator()(const T& value) const -> result_type {
        return value;
    }
};

}  // namespace

view_t::view_t(const value_t& value):
    inner(value.apply(into_view()))
{}

}  // namespace attribute
}  // namespace blackhole
