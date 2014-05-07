#pragma once

#include <memory>

#include "balancing.hpp"
#include "pool.hpp"

namespace elasticsearch {

class http_transport_t {
public:
    typedef pool_t<http_transport_t> pool_type;

private:
    pool_type pool;
    std::unique_ptr<balancing::strategy<pool_type>> balancer;
};

} // namespace elasticsearch
