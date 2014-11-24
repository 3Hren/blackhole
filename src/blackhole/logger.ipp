#include "blackhole/logger.hpp"

#include "blackhole/keyword.hpp"
#include "blackhole/keyword/message.hpp"
#include "blackhole/keyword/thread.hpp"
#include "blackhole/keyword/timestamp.hpp"
#include "blackhole/keyword/tracebit.hpp"
#include "blackhole/keyword/process.hpp"

namespace blackhole {

template<class T, class... FilterArgs>
void
composite_logger_t<T, FilterArgs...>::populate(attribute::set_t& internal) const {
    internal.reserve(BLACKHOLE_INTERNAL_SET_RESERVED_SIZE);

#ifdef BLACKHOLE_HAS_ATTRIBUTE_PID
    internal.emplace_back(keyword::pid() = keyword::init::pid());
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_TID
    internal.emplace_back(keyword::tid() = keyword::init::tid());
#endif

#ifdef BLACKHOLE_HAS_ATTRIBUTE_LWP
    internal.emplace_back(keyword::lwp() = keyword::init::lwp());
#endif

    internal.emplace_back(keyword::timestamp() = keyword::init::timestamp());
}

} // namespace blackhole
