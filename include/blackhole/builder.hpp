#pragma once

#include <memory>
#include <vector>

#include "forward.hpp"

#include "handler.hpp"

// TODO: Temporary. Split into formatter/string/builder & formatter/string/factory etc.
#include "formatter.hpp"
#include "formatter/string.hpp"
#include "sink/console.hpp"
#include "handler/blocking.hpp"
#include "sink.hpp"
#include "root.hpp"

namespace blackhole {
inline namespace v1 {

namespace experimental {

struct sink_tag;
struct filter_tag;
struct logger_tag;
struct handler_tag;
struct formatter_tag;

template<typename T, typename Parent = void>
class partial_builder;

template<>
class partial_builder<root_logger_t> {
public:
    typedef partial_builder<root_logger_t> this_type;

private:
    template<typename, typename> friend class partial_builder;

    std::vector<std::unique_ptr<handler_t>> handlers;

public:
    partial_builder() = default;

    explicit partial_builder(std::vector<std::unique_ptr<handler_t>> handlers) noexcept :
        handlers(std::move(handlers))
    {}

    template<typename T>
    auto handler() && -> partial_builder<T, this_type> {
        return {std::move(*this)};
    }

    auto build() && -> root_logger_t {
        return {std::move(*this).handlers};
    }
};

template<typename Parent>
class partial_builder<handler::blocking_t, Parent> {
public:
    typedef partial_builder<handler::blocking_t, Parent> this_type;

private:
    template<typename, typename> friend class partial_builder;

    Parent parent;
    builder<handler::blocking_t> builder;

public:
    partial_builder(Parent parent) noexcept :
        parent(std::move(parent))
    {}

    template<typename T, typename... Args>
    auto set(Args&&... args) && -> partial_builder<T, this_type> {
        return {std::move(*this), std::forward<Args>(args)...};
    }

    template<typename T, typename... Args>
    auto add(Args&&... args) && -> partial_builder<T, this_type> {
        return {std::move(*this), std::forward<Args>(args)...};
    }

    auto build() && -> Parent&& {
        parent.handlers.emplace_back(std::move(builder).build());
        return std::move(*this).parent;
    }
};

template<typename Parent>
class partial_builder<formatter::string_t, Parent> {
public:
    typedef Parent parent_type;
    typedef formatter::string_t internal_type;
    typedef partial_builder<internal_type, parent_type> this_type;

private:
    parent_type parent;
    builder<internal_type> builder;

public:
    partial_builder(parent_type parent, std::string pattern) noexcept :
        parent(std::move(parent)),
        builder(std::move(pattern))
    {}

    auto mapping(formatter::severity_map sevmap) && -> this_type&& {
        builder.mapping(std::move(sevmap));
        return std::move(*this);
    }

    auto build() && -> parent_type&& {
        parent.builder.set(std::move(builder).build());
        return std::move(*this).parent;
    }
};

template<typename Parent>
class partial_builder<sink::console_t, Parent> {
public:
    typedef Parent parent_type;
    typedef sink::console_t internal_type;
    typedef partial_builder<internal_type, parent_type> this_type;

private:
    parent_type parent;
    builder<internal_type> builder;

public:
    partial_builder(parent_type parent) noexcept :
        parent(std::move(parent))
    {}

    auto build() && -> parent_type&& {
        parent.builder.add(std::move(builder).build());
        return std::move(*this).parent;
    }
};

}  // namespace experimental
}  // namespace v1
}  // namespace blackhole
