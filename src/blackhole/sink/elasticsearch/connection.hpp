#pragma once

#include <functional>

#include <boost/asio.hpp>

namespace elasticsearch {

template<class Action>
struct callback {
    typedef std::function<void(typename Action::result_type)> type;
};

class http_connection_t {
public:
    typedef boost::asio::io_service loop_type;
    typedef boost::asio::ip::tcp protocol_type;
    typedef protocol_type::endpoint endpoint_type;

private:
    endpoint_type endpoint_;

public:
    http_connection_t(endpoint_type endpoint, loop_type&) :
        endpoint_(endpoint)
    {}

    endpoint_type endpoint() const {
        return endpoint_;
    }

    template<class Action>
    void perform(Action&&, typename callback<Action>::type) {}
};

} // namespace elasticsearch
