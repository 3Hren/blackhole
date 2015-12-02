#include "blackhole/scoped.hpp"

#include <boost/thread/tss.hpp>

namespace blackhole {

using attribute::view_t;

namespace {

// TODO: Replace somewhere near `view_of`.
auto transform(const attributes_t& source) -> attribute_list {
    attribute_list result;
    for (const auto& attribute : source) {
        result.emplace_back(attribute.first, view_t::from(attribute.second));
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

auto
scoped_t::collect(attribute_pack* pack) const -> void {
    pack->emplace_back(attributes);

    if (prev) {
        prev->collect(pack);
    }
}

auto
scoped_t::rebind(boost::thread_specific_ptr<scoped_t>* context) -> void {
    this->context = context;
    if (prev) {
        prev->rebind(context);
    }
}

}  // namespace blackhole
