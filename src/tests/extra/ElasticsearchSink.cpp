#include <blackhole/sink/elasticsearch.hpp>
#include <blackhole/utils/atomic.hpp>

#include "../global.hpp"

using namespace blackhole;

TEST(elasticsearch_t, Class) {
    using blackhole::sink::elasticsearch_t;
    elasticsearch_t sink;
    UNUSED(sink);
}

TEST(elasticsearch_t, Manual) {
    using blackhole::sink::elasticsearch_t;
    elasticsearch_t sink;
    std::string msg = "{}";
    boost::algorithm::replace_all(msg, "'", "\"");
    for (int i = 0; i < 200; ++i) {
        sink.consume(msg);
    }
}

using namespace elasticsearch;

namespace mock {

class logger_t {
public:
    template<typename... Args>
    log::record_t open_record(Args&&...) const {
        return log::record_t();
    }

    void push(log::record_t&&) const {}
};

class response_t {
};

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

class transport_t_SuccessfullyHandleMessage_Test;

namespace inspector {

//!@note: Helper class to easy ancestor's mock fields inspection.
template<class Connection, class Pool>
class http_transport_t : public elasticsearch::http_transport_t<Connection, Pool> {
    friend class ::transport_t_SuccessfullyHandleMessage_Test;

public:
    template<typename... Args>
    http_transport_t(Args&&... args) :
        elasticsearch::http_transport_t<Connection, Pool>(std::forward<Args>(args)...)
    {}
};

} // namespace inspector

template<class Action>
struct event_t {
    std::atomic<int>& counter;

    void operator()(typename Action::result_type) {
        counter++;
    }
};

TEST(transport_t, SuccessfullyHandleMessage) {
    boost::asio::io_service loop;
    synchronized<logger_base_t> log(logger_factory_t::create());

    std::unique_ptr<mock::balancer> balancer(new mock::balancer);
    std::shared_ptr<mock::connection_t> connection(new mock::connection_t);

    settings_t settings;
    inspector::http_transport_t<
        mock::connection_t,
        mock::pool_t
    > transport(settings, loop, log);
    EXPECT_CALL(*balancer, next(_))
            .Times(1)
            .WillOnce(Return(connection));
    EXPECT_CALL(*connection, endpoint())
            .Times(1)
            .WillOnce(Return(mock::connection_t::endpoint_type()));
    EXPECT_CALL(*connection, perform(An<mock::action_t>(), _, _))
            .Times(1)
            .WillOnce(
                InvokeArgument<1>(
                    result_t<mock::response_t>::type(mock::response_t())
                )
            );

    transport.balancer = std::move(balancer);

    std::atomic<int> counter(0);
    event_t<mock::action_t> event { counter };
    transport.perform(mock::action_t(), event);
    loop.run();

    EXPECT_EQ(1, counter);
}
