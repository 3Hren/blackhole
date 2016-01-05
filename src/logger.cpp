#include "blackhole/logger.hpp"

#include <boost/thread/tss.hpp>

#include "blackhole/attribute.hpp"

namespace blackhole {

logger_t::scoped_t::scoped_t(logger_t& logger) :
    context(logger.context()),
    prev(context->get())
{
    context->reset(this);
}

logger_t::scoped_t::~scoped_t() {
    BOOST_ASSERT(context);
    BOOST_ASSERT(context->get() == this);
    context->reset(prev);
}

auto logger_t::scoped_t::collect(attribute_pack* pack) const -> void {
    pack->emplace_back(attributes());

    if (prev) {
        prev->collect(pack);
    }
}

auto logger_t::scoped_t::rebind(boost::thread_specific_ptr<scoped_t>* context) -> void {
    this->context = context;
    if (prev) {
        prev->rebind(context);
    }
}

logger_t::~logger_t() = default;

}  // namespace blackhole
