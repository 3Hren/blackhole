#pragma once

#include <memory>

namespace elasticsearch {

namespace balancing {

template<class Pool>
class strategy {
public:
    typedef Pool pool_type;
    typedef typename Pool::connection_type connection_type;

    virtual std::shared_ptr<connection_type>& next(pool_type& pool) = 0;
};

template<class Pool>
class round_robin : public strategy<Pool> {
public:
    typedef typename strategy<Pool>::pool_type pool_type;
    typedef typename pool_type::connection_type connection_type;
    typedef typename pool_type::size_type size_type;

private:
    size_type current;

public:
    round_robin() :
        current(0)
    {}

    std::shared_ptr<connection_type>& next(pool_type& pool) {
        BOOST_ASSERT(!pool.empty());

        if (current >= pool.size() - 1) {
            current = 0;
        }

        auto it = std::next(pool.begin(), current++);
        std::shared_ptr<connection_type>& connection = it->second;
//        BH_LOG(log(), level::debug, "balancing at %s", connection->endpoint());
        return connection;
    }
};

} // namespace balancing

} // namespace elasticsearch
