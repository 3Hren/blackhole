#pragma once

#include <chrono>
#include <string>
#include <thread>

#include <boost/type_traits/remove_cv.hpp>
#include <boost/variant/apply_visitor.hpp>

#include "blackhole/attribute.hpp"
#include "blackhole/attributes.hpp"
#include "blackhole/extensions/writer.hpp"
#include "blackhole/record.hpp"
#include "blackhole/severity.hpp"

#include "blackhole/detail/attribute.hpp"
#include "blackhole/detail/record.hpp"

// TODO: Move `into_view` and `from_view` traits.

namespace blackhole {
inline namespace v1 {
namespace detail {

struct into_owned_t {
    typedef attribute::value_t result_type;

    template<typename T>
    auto operator()(T value) const -> result_type {
        return {value};
    }

    auto operator()(const attribute::view_t::string_type& value) const -> result_type {
        return {value.to_string()};
    }

    // TODO: Consider mapping into function.
    auto operator()(const attribute::view_t::function_type& value) const -> result_type {
        writer_t wr;
        value(wr);
        return {wr.result().to_string()};
    }
};

/// Represents a trait for mapping an owned types to their associated lightweight view types.
// TODO: Partially copies the same trait from `blackhole::v1` namespace.
template<typename T>
struct view_of;

template<>
struct view_of<std::string> {
    typedef string_view type;
    typedef std::string buffer_type;

    static auto from(const buffer_type& value) -> type {
        return {value};
    }
};

template<>
struct view_of<attributes_t> {
    typedef attribute_list type;
    typedef attributes_t buffer_type;

    static auto from(const buffer_type& value) -> type {
        type result;
        std::copy(std::begin(value), std::end(value), std::back_inserter(result));
        return result;
    }
};

template<typename T>
struct owned_pair {
    T data;
    typename view_of<T>::type view;

    explicit owned_pair(T value) :
        data(std::move(value)),
        view(view_of<T>::from(data))
    {}

    owned_pair(const owned_pair& other) = delete;
    owned_pair(owned_pair&& other) :
        data(std::move(other.data)),
        view(view_of<T>::from(data))
    {}

    auto operator=(const owned_pair& other) -> owned_pair& = delete;
    auto operator=(owned_pair&& other) -> owned_pair& {
        if (this != &other) {
            data = std::move(other.data);
            view = std::move(other.view);
            view = view_of<T>::from(data);
        }

        return *this;
    }
};

/// An owned, mutable record.
class recordbuf_t {
    owned_pair<std::string> message;
    owned_pair<std::string> formatted;
    owned_pair<attributes_t> attributes;

    attribute_pack pack;

    typedef std::aligned_storage<sizeof(record_t::inner_t)>::type storage_type;
    storage_type storage;

public:
    /// Constructs an empty invalid record.
    ///
    /// Required only by MPSC queue API. All access to such object will likely result in segfault.
    recordbuf_t() : message(""), formatted(""), attributes({}) {}

    /// Converts a record to an owned recordbuf.
    ///
    /// \throw std::bad_alloc on memory allocation failure.
    explicit recordbuf_t(const record_t& record) :
        message(record.message().to_string()),
        formatted(record.formatted().to_string()),
        attributes(flatten(record.attributes()))
    {
        pack.emplace_back(attributes.view);

        auto& inner = this->inner();
        inner.message = message.view;
        inner.formatted = formatted.view;
        inner.severity = record.severity();
        inner.timestamp = record.timestamp();
        inner.tid = record.tid();
        inner.attributes = pack;
    }

    recordbuf_t(const recordbuf_t& other) = delete;

    recordbuf_t(recordbuf_t&& other) noexcept :
        message(std::move(other.message)),
        formatted(std::move(other.formatted)),
        attributes(std::move(other.attributes)),
        storage(other.storage),
        pack(std::move(other.pack))
    {
        BOOST_ASSERT(pack.size() == 1);
        pack.back() = attributes.view;

        auto& inner = this->inner();
        inner.message = message.view;
        inner.formatted = formatted.view;
        inner.attributes = pack;
    }

    auto operator=(const recordbuf_t& other) -> recordbuf_t& = delete;

    auto operator=(recordbuf_t&& other) noexcept -> recordbuf_t& {
        if (this != &other) {
            message = std::move(other.message);
            formatted = std::move(other.formatted);
            attributes = std::move(other.attributes);
            storage = other.storage;
            pack = std::move(other.pack);

            BOOST_ASSERT(pack.size() == 1);
            pack.back() = attributes.view;

            auto& inner = this->inner();
            inner.message = message.view;
            inner.formatted = formatted.view;
            inner.attributes = pack;
        }

        return *this;
    }

    auto into_view() const noexcept -> record_t {
        return {inner()};
    }

private:
    auto inner() noexcept -> record_t::inner_t& {
        return reinterpret_cast<record_t::inner_t&>(storage);
    }

    auto inner() const noexcept -> const record_t::inner_t& {
        return reinterpret_cast<const record_t::inner_t&>(storage);
    }

    static auto flatten(const attribute_pack& pack) -> attributes_t {
        attributes_t result;

        for (auto list : pack) {
            for (auto kv : list.get()) {
                result.emplace_back(
                    kv.first.to_string(),
                    boost::apply_visitor(into_owned_t(), kv.second.inner().value)
                );
            }
        }

        return result;
    }
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
