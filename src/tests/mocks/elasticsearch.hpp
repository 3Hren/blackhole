#pragma once

using namespace elasticsearch;

namespace mock {

class response_t {};

class action_t {
public:
    typedef response_t response_type;
    typedef result_t<response_type>::type result_type;

    static const request::method_t method_value = request::method_t::get;

    static const char* name() {
        return "mock.action";
    }

    std::string path() const {
        return "/";
    }
};

class connection_t {
public:
    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::endpoint endpoint_type;

    template<class... Args>
    connection_t(Args&&...) {}

    MOCK_CONST_METHOD0(endpoint, endpoint_type());

    MOCK_METHOD3(perform, void(
        actions::nodes_info_t,
        callback<actions::nodes_info_t>::type,
        long
    ));

    MOCK_METHOD3(perform, void(
        action_t,
        callback<action_t>::type,
        long
    ));
};

class pool_t {
public:
    typedef connection_t connection_type;
    typedef connection_type::endpoint_type endpoint_type;
    typedef std::unordered_map<
        endpoint_type,
        std::shared_ptr<connection_type>
    > pool_type;
    typedef pool_type::size_type size_type;
    typedef pool_type::iterator iterator;

    typedef std::mutex mutex_type;
    typedef pool_lock_t<pool_t> pool_lock_type;

    mutable std::mutex mutex;

    typedef std::pair<iterator, bool> pair_type;
    MOCK_METHOD2(insert, pair_type(
        const endpoint_type&,
        const std::shared_ptr<connection_type>&
    ));
    MOCK_METHOD1(remove, void(const endpoint_type&));

    //!@note: These methods are pure stubs, they shouldn't be called ever.
    MOCK_CONST_METHOD1(size, size_type(pool_lock_type&));
    MOCK_CONST_METHOD1(empty, bool(pool_lock_type&));
    MOCK_METHOD1(begin, iterator(pool_lock_type&));
    MOCK_METHOD1(end, iterator(pool_lock_type&));
};

class balancer : public balancing::strategy<pool_t> {
public:
    typedef pool_t pool_type;
    typedef pool_type::connection_type connection_type;

    MOCK_METHOD1(next, std::shared_ptr<connection_type>(pool_type& pool));
};

} // namespace mock

namespace elasticsearch {

template<>
struct extractor_t<mock::response_t> {
    static mock::response_t extract(const rapidjson::Value&) {
        return mock::response_t();
    }
};

} // namespace elasticsearch
