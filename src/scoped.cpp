#include "blackhole/scoped.hpp"

namespace blackhole {

namespace {

auto transform(const attributes_t& source) -> attribute_list {
    attribute_list result;
    for (const auto& attribute : source) {
        result.emplace_back(attribute);
    }

    return result;
}

}  // namespace

scoped_t::scoped_t(boost::thread_specific_ptr<scoped_t>& prev, attributes_t attributes):
    scoped(prev),
    prev(scoped.get()),
    storage(std::move(attributes)),
    attributes(transform(storage))
{
    scoped.reset(this);
}

scoped_t::~scoped_t() {
    BOOST_ASSERT(scoped);
    BOOST_ASSERT(scoped.get() == this);
    scoped.reset(prev);
}

}  // namespace blackhole
