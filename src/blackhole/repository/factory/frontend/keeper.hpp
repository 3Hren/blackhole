#pragma once

#include <unordered_map>

#include <boost/any.hpp>

#include "blackhole/repository/factory/frontend/helper.hpp"
#include "blackhole/repository/factory/frontend/traits.hpp"

namespace blackhole {

/*!
 * Keeps frontend factory functions using type erasure idiom.
 * For each desired formatter/sink pair a function is created and registered.
 * Later that function can be extracted by the sink type parameter.
*/
struct function_keeper_t {
    std::unordered_map<std::string, boost::any> functions;

    template<class Sink, class Formatter>
    void add() {
        typedef typename frontend::traits<Sink>::function_type function_type;
        //!@todo: There is an issue, the `Sink::name()` should provide unique
        //!       name to be able to separate the same sinks with multiple
        //!       inner configuration.
        functions[Sink::name()] = static_cast<
            function_type
        >(&factory::frontend::create<Formatter>);
    }

    template<class Sink>
    bool has() const {
        return functions.find(Sink::name()) != functions.end();
    }

    template<class Sink>
    typename frontend::traits<Sink>::function_type
    get() const {
        typedef typename frontend::traits<Sink>::function_type function_type;
        return boost::any_cast<function_type>(functions.at(Sink::name()));
    }
};

} // namespace blackhole
