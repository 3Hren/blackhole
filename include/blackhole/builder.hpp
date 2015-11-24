#pragma once

#include <memory>
#include <vector>

namespace blackhole {

template<typename> class builder; // temp.
class handler_t;
class logger_t;

}

namespace blackhole {

template<typename L, typename T>
class handler_builder {
public:
    typedef L logger_type;
    typedef T handler_type;
    typedef builder<logger_type> parent_type;
    typedef handler_builder<logger_type, handler_type> self_type;

private:
    parent_type& parent;
    std::unique_ptr<handler_type> handler;
    std::vector<std::unique_ptr<handler_t>>& handlers;

public:
    constexpr
    handler_builder(parent_type& parent, std::unique_ptr<handler_type> handler,
        std::vector<std::unique_ptr<handler_t>>& handlers) noexcept :
        parent(parent),
        handler(std::move(handler)),
        handlers(handlers)
    {}

    template<typename F, typename... Args>
    auto set(Args&&... args) -> self_type& {
        handler->set(std::unique_ptr<F>(new F(std::forward<Args>(args)...)));
        return *this;
    }

    template<typename S, typename... Args>
    auto add(Args&&... args) -> self_type& {
        handler->add(std::unique_ptr<S>(new S(std::forward<Args>(args)...)));
        return *this;
    }

    auto build() noexcept -> parent_type& {
        // TODO: Check handler correctness. It's possible to build without formatter or sinks.
        // Can be fixed using strong-typed builder.
        handlers.emplace_back(std::move(handler));
        return parent;
    }
};

template<typename L>
class builder {
public:
    typedef L logger_type;

private:
    std::vector<std::unique_ptr<handler_t>> handlers;

public:
    template<typename H, typename... Args>
    auto handler(Args&&... args) -> handler_builder<logger_type, H> {
        return handler_builder<logger_type, H>{
            *this,
            std::unique_ptr<H>(new H(std::forward<Args>(args)...)),
            handlers
        };
    }

    // TODO: Filter.

    auto build() -> logger_type {
        return {std::move(handlers)};
    }
};

}  // namespace blackhole
