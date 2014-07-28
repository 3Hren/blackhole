#pragma once

#include <memory>

#include "blackhole/forwards.hpp"

namespace blackhole {

namespace frontend {

template<class T>
struct traits {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef return_type(*function_type)(const formatter_config_t&, std::unique_ptr<T>);
};

} // namespace frontend

} // namespace blackhole
