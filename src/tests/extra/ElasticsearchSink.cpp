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
class transport_t_HandleGenericError_Test;
class transport_t_HandleConnectionErrorWhenSniffOnErrorIsFalse_Test;
class transport_t_HandleConnectionErrorWhenSniffOnErrorIsFalseAndNoConnectionsLeft_Test;

namespace inspector {

//!@note: Helper class to easy ancestor's mock fields inspection.
template<class Connection, class Pool>
class http_transport_t : public elasticsearch::http_transport_t<Connection, Pool> {
    friend class ::transport_t_SuccessfullyHandleMessage_Test;
    friend class ::transport_t_HandleGenericError_Test;
    friend class ::transport_t_HandleConnectionErrorWhenSniffOnErrorIsFalse_Test;
    friend class ::transport_t_HandleConnectionErrorWhenSniffOnErrorIsFalseAndNoConnectionsLeft_Test;

public:
    template<typename... Args>
    http_transport_t(Args&&... args) :
        elasticsearch::http_transport_t<Connection, Pool>(std::forward<Args>(args)...)
    {}
};

} // namespace inspector

template<class Action, class T>
struct event_t {
    std::atomic<int>& counter;

    void operator()(typename Action::result_type result) {
        counter++;
        EXPECT_TRUE(boost::get<T>(&result));
    }
};

void post(boost::asio::io_service& loop,
          callback<mock::action_t>::type callback,
          result_t<mock::response_t>::type result) {
    loop.post(std::bind(callback, result));
}

namespace stub {

synchronized<logger_base_t> log(logger_factory_t::create());

} // namespace stub

TEST(transport_t, SuccessfullyHandleMessage) {
    boost::asio::io_service loop;

    std::unique_ptr<mock::balancer> balancer(new mock::balancer);
    std::shared_ptr<mock::connection_t> connection(new mock::connection_t);

    settings_t settings;

    inspector::http_transport_t<
        mock::connection_t,
        mock::pool_t
    > transport(settings, loop, stub::log);

    EXPECT_CALL(*balancer, next(_))
            .Times(1)
            .WillOnce(Return(connection));
    EXPECT_CALL(*connection, endpoint())
            .WillOnce(Return(mock::connection_t::endpoint_type()));
    EXPECT_CALL(*connection, perform(An<mock::action_t>(), _, _))
            .Times(1)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post,
                            std::ref(loop),
                            std::placeholders::_1,
                            mock::response_t()
                        )
                    )
                )
            );

    transport.balancer = std::move(balancer);

    std::atomic<int> counter(0);
    transport.perform(
        mock::action_t(),
        event_t<mock::action_t, mock::response_t> { counter }
    );
    loop.run_one();

    EXPECT_EQ(1, counter);
}

TEST(transport_t, HandleGenericError) {
    /*! After receiving the response with some elasticsearch error, a caller's
     *  callback must be called during the next event loop tick.
     */

    boost::asio::io_service loop;

    std::unique_ptr<mock::balancer> balancer(new mock::balancer);
    std::shared_ptr<mock::connection_t> connection(new mock::connection_t);

    settings_t settings;

    inspector::http_transport_t<
        mock::connection_t,
        mock::pool_t
    > transport(settings, loop, stub::log);

    EXPECT_CALL(*balancer, next(_))
            .Times(1)
            .WillOnce(Return(connection));
    EXPECT_CALL(*connection, endpoint())
            .WillOnce(Return(mock::connection_t::endpoint_type()));
    EXPECT_CALL(*connection, perform(An<mock::action_t>(), _, _))
            .Times(1)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post,
                            std::ref(loop),
                            std::placeholders::_1,
                            elasticsearch::error_t(generic_error_t("mock"))
                        )
                    )
                )
            );

    transport.balancer = std::move(balancer);

    std::atomic<int> counter(0);
    transport.perform(
        mock::action_t(),
        event_t<mock::action_t, elasticsearch::error_t> { counter }
    );
    loop.run_one();

    EXPECT_EQ(1, counter);
}

TEST(transport_t, HandleConnectionErrorWhenSniffOnErrorIsFalse) {
    /*! After receiving connection error, transport should remove the corrupted
     *  node from the pool and try another endpoint.
     *  Sniffing after error shouldn't occur, because we disable this option
     *  explicitly.
     *  The second request succeed.
     */

    boost::asio::io_service loop;

    std::unique_ptr<mock::balancer> balancer(new mock::balancer);
    std::shared_ptr<mock::connection_t> connection(new mock::connection_t);

    settings_t settings;
    settings.sniffer.when.error = false;

    inspector::http_transport_t<
        mock::connection_t,
        mock::pool_t
    > transport(settings, loop, stub::log);

    EXPECT_CALL(*balancer, next(_))
            .Times(2)
            .WillRepeatedly(Return(connection));
    EXPECT_CALL(*connection, endpoint())
            .WillRepeatedly(Return(mock::connection_t::endpoint_type()));
    EXPECT_CALL(transport.pool, remove(mock::connection_t::endpoint_type()))
            .Times(1);
    EXPECT_CALL(*connection, perform(An<mock::action_t>(), _, _))
            .Times(2)
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post,
                            std::ref(loop),
                            std::placeholders::_1,
                            elasticsearch::error_t(
                                connection_error_t(
                                    mock::connection_t::endpoint_type(),
                                    "mock"
                                )
                            )
                        )
                    )
                )
            )
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post,
                            std::ref(loop),
                            std::placeholders::_1,
                            mock::response_t()
                        )
                    )
                )
            );

    transport.balancer = std::move(balancer);

    std::atomic<int> counter(0);
    transport.perform(
        mock::action_t(),
        event_t<mock::action_t, mock::response_t> { counter }
    );
    loop.run_one();

    //! Poll the second callback (should be successful).
    loop.run_one();

    EXPECT_EQ(1, counter);
}

TEST(transport_t, HandleConnectionErrorWhenSniffOnErrorIsFalseAndNoConnectionsLeft) {
    /*! After receiving connection error, transport should remove the corrupted
     *  node from the pool and try another endpoint.
     *  Sniffing after error shouldn't occur, because we disable this option
     *  explicitly.
     *  The second request will fail, because there is no connections left.
     */

    boost::asio::io_service loop;

    std::unique_ptr<mock::balancer> balancer(new mock::balancer);
    std::shared_ptr<mock::connection_t> connection(new mock::connection_t);

    settings_t settings;
    settings.sniffer.when.error = false;

    inspector::http_transport_t<
        mock::connection_t,
        mock::pool_t
    > transport(settings, loop, stub::log);

    EXPECT_CALL(*balancer, next(_))
            .Times(2)
            .WillOnce(Return(connection))
            // The second balancing attepmt should return no connection,
            // because our pool is empty.
            .WillOnce(Return(std::shared_ptr<mock::connection_t>()));
    EXPECT_CALL(*connection, endpoint())
            .WillRepeatedly(Return(mock::connection_t::endpoint_type()));
    EXPECT_CALL(transport.pool, remove(mock::connection_t::endpoint_type()))
            .Times(1);
    EXPECT_CALL(*connection, perform(An<mock::action_t>(), _, _))
            .Times(1)
            // Here we mock first perform to be failed with connection error.
            .WillOnce(
                WithArg<1>(
                    Invoke(
                        std::bind(
                            &post,
                            std::ref(loop),
                            std::placeholders::_1,
                            elasticsearch::error_t(
                                connection_error_t(
                                    mock::connection_t::endpoint_type(),
                                    "mock"
                                )
                            )
                        )
                    )
                )
            );

    transport.balancer = std::move(balancer);

    std::atomic<int> counter(0);
    transport.perform(
        mock::action_t(),
        event_t<mock::action_t, elasticsearch::error_t> { counter }
    );
    loop.run_one();
    loop.run_one();

    EXPECT_EQ(1, counter);
}

