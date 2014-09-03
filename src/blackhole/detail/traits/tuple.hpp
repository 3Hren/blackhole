#pragma once

#include "tuple/all.hpp"
#include "tuple/concat.hpp"
#include "tuple/filter.hpp"
#include "tuple/index/add.hpp"
#include "tuple/index/remove.hpp"
#include "tuple/map.hpp"
#include "tuple/slice.hpp"

/// A type that represents a parameter pack of zero or more integers.
template<unsigned... indices>
struct index_tuple {
    /// Generate an index_tuple with an additional element.
    template<unsigned N>
    struct append {
        typedef index_tuple<indices..., N> type;
    };
};

/// Unary metafunction that generates an index_tuple containing [0, Size)
template<unsigned size>
struct make_index_tuple {
    typedef typename make_index_tuple<size - 1>::type::template append<size - 1>::type type;
};

/// Terminal case of the recursive metafunction.
template<>
struct make_index_tuple<0u> {
    typedef index_tuple<> type;
};
