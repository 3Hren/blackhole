#pragma once

#include <type_traits>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/variant.hpp>

namespace elasticsearch {

struct connection_error_t {
    typedef boost::asio::ip::tcp::endpoint endpoint_type;

    endpoint_type endpoint;
    std::string reason;

    connection_error_t(endpoint_type endpoint, const std::string& reason) :
        endpoint(endpoint),
        reason(reason)
    {}
};

struct generic_error_t {
    std::string reason;

    generic_error_t(const std::string& reason) :
        reason(reason)
    {}
};

typedef boost::variant<connection_error_t, generic_error_t> error_t;

template<class T>
struct result_t {
    typedef boost::variant<T, error_t> type;
};

} // namespace elasticsearch
