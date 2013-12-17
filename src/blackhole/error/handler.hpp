#pragma once

#include <functional>
#include <stdexcept>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/reverse.hpp>

#include "blackhole/config.hpp"

namespace blackhole {

namespace log {

typedef std::function<void()> exception_handler_t;

class default_exception_handler_t {
public:
    void operator()() const {
#ifdef BLACKHOLE_DEBUG
        throw;
#else
        try {
            throw;
        } catch (const std::exception& err) {
            std::cout << "logging core error occurred: " << err.what() << std::endl;
        } catch (...) {
            std::cout << "logging core error occurred: unknown" << std::endl;
        }
#endif
    }
};

namespace aux {

namespace vector {

template<typename... Args>
struct from_variadic;

template<typename T, typename... Args>
struct from_variadic<T, Args...> {
    typedef typename boost::mpl::push_back<
        typename from_variadic<Args...>::type,
        T
    >::type type;
};

template<typename T>
struct from_variadic<T> {
    typedef boost::mpl::vector<T> type;
};

template<>
struct from_variadic<> {
    typedef boost::mpl::vector<> type;
};

} // namespace vector

} // namespace aux

namespace aux {

template<typename Handler>
class launcher {
protected:
    typedef Handler handler_type;
    handler_type handler;

public:
    launcher(const handler_type& handler) :
        handler(handler)
    {}

    void operator()() {
        throw;
    }
};

template<typename Exception, typename Base>
class intermediate : public Base {
protected:
    typedef typename Base::handler_type handler_type;

public:
    intermediate(const handler_type& handler) :
        Base(handler)
    {}

    void operator()() {
        try {
            Base::operator()();
        } catch (const Exception& err) {
            this->handler(err);
        }
    }
};

} // namespace aux

template<typename TypeList, typename Handler>
class exception_handler :
    public boost::mpl::fold<
        TypeList,
        aux::launcher<Handler>,
        boost::mpl::bind<boost::mpl::quote2<aux::intermediate>, boost::mpl::_2, boost::mpl::_1>
    >::type {

    typedef typename boost::mpl::fold<
        TypeList,
        aux::launcher<Handler>,
        boost::mpl::bind<boost::mpl::quote2<aux::intermediate>, boost::mpl::_2, boost::mpl::_1>
    >::type base_type;

public:
    exception_handler(Handler handler) :
        base_type(handler)
    {}

    void operator()() {
        base_type::operator()();
    }
};

namespace exception {

template<typename Handler>
struct handler_factory_t {
    template<typename... Args>
    static
    exception_handler<
        typename boost::mpl::reverse<
            typename aux::vector::from_variadic<Args...>::type
        >::type,
        Handler
    > make() {
        typedef exception_handler<
            typename boost::mpl::reverse<
                typename aux::vector::from_variadic<Args...>::type
            >::type,
            Handler
        > handler_type;
        return handler_type(Handler());
    }
};

} // namespace exception

} // namespace log

} // namespace blackhole
