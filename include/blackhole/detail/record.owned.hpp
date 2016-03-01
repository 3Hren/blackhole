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
class owned;

// TODO: Consider better naming for both class and file.
template<>
class owned<record_t> {
public:
    typedef record_t view_type;

private:
    typedef record_t::inner_t inner_t;

    std::string message;
    string_view message_view;

    std::string formatted;
    string_view formatted_view;

    attributes_t attributes;
    attribute_list attributes_view;
    attribute_pack attributes_pack;

    typedef std::aligned_storage<64>::type storage_type;
    storage_type storage;

public:
    explicit owned(const record_t& record) :
        message(record.message().to_string()),
        message_view(message),
        formatted(record.formatted().to_string()),
        formatted_view(formatted)
    {
        auto& inner = this->inner();

        inner.message = message_view;
        inner.formatted = formatted_view;

        inner.severity = record.severity();
        inner.timestamp = record.timestamp();

        inner.tid = record.tid();

        for (auto list : record.attributes()) {
            for (auto kv : list.get()) {
                attributes.push_back({
                    kv.first.to_string(),
                    boost::apply_visitor(into_owned_t(), kv.second.inner().value)
                });
            }
        }

        for (const auto& attribute : attributes) {
            attributes_view.emplace_back(attribute);
        }

        attributes_pack.emplace_back(attributes_view);

        inner.attributes = attributes_pack;
    }

    owned(const owned& other) = delete;

    owned(owned&& other) :
        message(std::move(other.message)),
        message_view(message),
        formatted(std::move(other.formatted)),
        formatted_view(formatted),
        attributes(std::move(other.attributes)),
        storage(other.storage)
    {
        auto& inner = this->inner();

        inner.message = message_view;
        inner.formatted = formatted_view;

        for (const auto& attribute : attributes) {
            attributes_view.emplace_back(attribute);
        }

        attributes_pack.emplace_back(attributes_view);

        inner.attributes = attributes_pack;
    }

    auto operator=(const owned& other) -> owned& = delete;
    auto operator=(owned&& other) -> owned& = delete;

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
};

}  // namespace detail
}  // namespace v1
}  // namespace blackhole
