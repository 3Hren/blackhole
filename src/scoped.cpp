#include "blackhole/attribute.hpp"
#include "blackhole/logger.hpp"
#include "blackhole/scoped.hpp"

#include <boost/thread/tss.hpp>

namespace blackhole {

scoped_t::scoped_t(logger_t& logger) :
    context(logger.context()),
    prev(context->get())
{
    context->reset(this);
}

scoped_t::~scoped_t() {
    BOOST_ASSERT(context);
    BOOST_ASSERT(context->get() == this);
    context->reset(prev);
}

auto scoped_t::collect(attribute_pack* pack) const -> void {
    pack->emplace_back(attributes());

    if (prev) {
        prev->collect(pack);
    }
}

auto scoped_t::rebind(boost::thread_specific_ptr<scoped_t>* context) -> void {
    this->context = context;
    if (prev) {
        prev->rebind(context);
    }
}

}  // namespace blackhole
