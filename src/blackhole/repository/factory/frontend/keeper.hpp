#pragma once

#include <unordered_map>

#include <boost/any.hpp>

#include "blackhole/repository/factory/frontend/helper.hpp"
#include "blackhole/repository/factory/frontend/traits.hpp"

namespace blackhole {

//! \brief Keeps frontend factory functions using type erasure idiom.
/*! For each desired Formatter-Sink pair a function created and registered.
    Later that function can be extracted by Sink type parameter.
*/
struct function_keeper_t {
    std::unordered_map<std::string, boost::any> functions;

    template<class Sink, class Formatter>
    void add() {
        typedef typename frontend::traits<Sink>::function_type function_type;
        function_type function = static_cast<function_type>(&factory::frontend::create<Formatter>);
        functions[Sink::name()] = function;
    }

    template<class Sink>
    bool has() const {
        return functions.find(Sink::name()) != functions.end();
    }

    template<class Sink>
    typename frontend::traits<Sink>::function_type
    get() const {
        return boost::any_cast<typename frontend::traits<Sink>::function_type>(
            functions.at(Sink::name())
        );
    }
};

} // namespace blackhole
