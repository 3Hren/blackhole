#pragma once

#include "blackhole/attribute.hpp"
#include "blackhole/detail/traits/attributes/pack/keyword.hpp"
#include "blackhole/detail/traits/attributes/pack/emplace.hpp"
#include "blackhole/detail/traits/attributes/pack/convert.hpp"
#include "blackhole/record.hpp"

namespace blackhole {

namespace aux {

struct keyword_pack_tag_t;
struct emplace_pack_tag_t;
struct unknown_pack_tag_t;

//! Helper metafunction that determines tag type of attributes pack.
//! At this moment only keyword and emplace pack are supported.
template<typename... Args>
struct pack_determiner :
    public std::conditional<
        is_keyword_pack<Args...>::value,
        keyword_pack_tag_t,
        typename std::conditional<
            is_emplace_pack<Args...>::value,
            emplace_pack_tag_t,
            unknown_pack_tag_t
        >::type
    >
{};

template<class T>
struct pack_feeder;

template<>
struct pack_feeder<keyword_pack_tag_t> {
    template<class T>
    static
    inline
    void
    feed(record_t& record, T&& arg) {
        record.insert(std::forward<T>(arg));
    }

    template<class Arg, class... Args>
    static
    inline
    void
    feed(record_t& record, Arg&& arg, Args&&... args) {
        feed(record, arg);
        feed(record, std::forward<Args>(args)...);
    }
};

template<>
struct pack_feeder<emplace_pack_tag_t> {
    template<class T>
    static
    inline
    void
    feed(record_t& record, const char* name, T&& value) {
        record.insert(
            std::make_pair(
                name,
                attribute_t(conv<T>::from(std::forward<T>(value)))
            )
        );
    }

    template<class T, class... Args>
    static
    inline
    void
    feed(record_t& record, const char* name, T&& value, Args&&... args) {
        feed(record, name, std::forward<T>(value));
        feed(record, std::forward<Args>(args)...);
    }
};

template<>
struct pack_feeder<unknown_pack_tag_t> {};

} // namespace aux

} // namespace blackhole
