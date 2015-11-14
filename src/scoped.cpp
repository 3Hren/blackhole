#include "blackhole/scoped.hpp"

#include <boost/thread/tss.hpp>

namespace blackhole {

namespace {

// TODO: Replace somewhere near `view_of`.
auto transform(const attributes_t& source) -> attribute_list {
    attribute_list result;
    for (const auto& attribute : source) {
        result.emplace_back(attribute);
    }

    return result;
}

}  // namespace

scoped_t::scoped_t(boost::thread_specific_ptr<scoped_t>* context, attributes_t attributes):
    prev(context->get()),
    context(context),
    storage(std::move(attributes)),
    attributes(transform(storage))
{
    context->reset(this);
}

scoped_t::~scoped_t() {
    BOOST_ASSERT(context);
    BOOST_ASSERT(context->get() == this);
    context->reset(prev);
}

}  // namespace blackhole
