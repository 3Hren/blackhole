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

    auto operator()(const attribute::view_t::function_type& value) const -> result_type {
        writer_t wr;
        value(wr);
        return {wr.result().to_string()};
    }
};

template<typename T>
struct twin_traits;

template<>
struct twin_traits<std::string> {
    typedef string_view view_type;

    static auto view_from(const std::string& value) -> view_type {
        return {value};
    }
};

template<>
struct twin_traits<attributes_t> {
    typedef attribute_list view_type;

    static auto view_from(const attributes_t& value) -> view_type {
        view_type result;

        for (const auto& item : value) {
            result.emplace_back(item);
        }

        return result;
    }
};

template<typename T>
struct twins {
    T data;
    typename twin_traits<T>::view_type view;

    explicit twins(T value) :
        data(std::move(value)),
        view(twin_traits<T>::view_from(data))
    {}

    twins(twins&& other) :
        data(std::move(other.data)),
        view(twin_traits<T>::view_from(data))
    {}

    auto operator=(twins&& other) -> twins& {
        if (this != &other) {
            data = std::move(other.data);
            view = twin_traits<T>::view_from(data);
        }

        return *this;
    }
};

template<typename T>
class owned;

// TODO: Consider better naming for both class and file.
template<>
class owned<record_t> {
public:
    typedef record_t view_type;

private:
    typedef record_t::inner_t inner_t;

    twins<std::string> message;
    twins<std::string> formatted;
    twins<attributes_t> attributes;

    attribute_pack pack;

    typedef std::aligned_storage<64>::type storage_type;
    storage_type storage;

public:
    explicit owned(const record_t& record) :
        message(record.message().to_string()),
        formatted(record.formatted().to_string()),
        attributes(flatten(record.attributes()))
    {
        pack = {{attributes.view}};

        auto& inner = this->inner();
        inner.message = message.view;
        inner.formatted = formatted.view;
        inner.severity = record.severity();
        inner.timestamp = record.timestamp();
        inner.tid = record.tid();
        inner.attributes = pack;
    }

    owned(const owned& other) = delete;

    owned(owned&& other) :
        message(std::move(other.message)),
        formatted(std::move(other.formatted)),
        attributes(std::move(other.attributes)),
        storage(other.storage)
    {
        pack = {{attributes.view}};

        auto& inner = this->inner();
        inner.message = message.view;
        inner.formatted = formatted.view;
        inner.attributes = pack;
    }

    auto operator=(const owned& other) -> owned& = delete;

    auto operator=(owned&& other) -> owned& {
        if (this != &other) {
            message = std::move(other.message);
            formatted = std::move(other.formatted);
            attributes = std::move(other.attributes);

            storage = other.storage;

            pack = {{attributes.view}};

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
    auto inner() noexcept -> inner_t& {
        return reinterpret_cast<inner_t&>(storage);
    }

    auto inner() const noexcept -> const inner_t& {
        return reinterpret_cast<const inner_t&>(storage);
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
