#pragma once

#include <memory>

namespace blackhole {

class base_frontend_t;
struct formatter_config_t;

namespace frontend {

template<class T>
struct traits {
    typedef std::unique_ptr<base_frontend_t> return_type;
    typedef return_type(*function_type)(const formatter_config_t&, std::unique_ptr<T>);
};

} // namespace frontend

} // namespace blackhole
