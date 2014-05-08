#pragma once

#include <unordered_map>

#include <boost/asio.hpp>

#include "blackhole/utils/noexcept.hpp"

namespace std {

template<>
struct hash<boost::asio::ip::tcp::endpoint> {
    typedef boost::asio::ip::tcp::endpoint argument_type;
    typedef std::size_t value_type;

    value_type operator()(argument_type const& value) const {
        value_type const h1(std::hash<std::string>()(value.address().to_string()));
        value_type const h2(std::hash<std::uint16_t>()(value.port()));
        return h1 ^ (h2 << 1);
    }
};

} // namespace std

namespace elasticsearch {

template<class Pool>
class pool_lock_t {
public:
    typedef typename Pool::mutex_type mutex_type;

private:
    std::lock_guard<mutex_type> lock;

public:
    pool_lock_t(Pool& pool) :
        lock(pool.mutex)
    {}
};

template<typename Connection>
class pool_t {
public:
    typedef Connection connection_type;

    typedef boost::asio::ip::tcp protocol_type;
    typedef typename protocol_type::endpoint endpoint_type;

    typedef std::unordered_map<
        endpoint_type,
        std::shared_ptr<connection_type>
    > pool_type;
    typedef typename pool_type::size_type size_type;
    typedef typename pool_type::iterator iterator;
    typedef typename pool_type::const_iterator const_iterator;

    typedef std::mutex mutex_type;
    friend class pool_lock_t<pool_t<Connection>>;

private:
    pool_type pool;
    mutable mutex_type mutex;

public:
    bool empty() const {
        return pool.empty();
    }

    size_type size() const {
        return pool.size();
    }

    std::pair<iterator, bool>
    insert(const endpoint_type& endpoint,
           const std::shared_ptr<connection_type>& connection) {
        std::lock_guard<mutex_type> lock(mutex);
        return pool.insert(std::make_pair(endpoint, connection));
    }

    void remove(const endpoint_type& endpoint) {
        pool.erase(endpoint);
    }

    iterator begin() BLACKHOLE_NOEXCEPT {
        return pool.begin();
    }

    iterator end() BLACKHOLE_NOEXCEPT {
        return pool.end();
    }

    const_iterator begin() const BLACKHOLE_NOEXCEPT {
        return pool.begin();
    }

    const_iterator end() const BLACKHOLE_NOEXCEPT {
        return pool.end();
    }
};

} // namespace elasticsearch
