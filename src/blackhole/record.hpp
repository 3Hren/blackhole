#pragma once

#include "attribute.hpp"
#include "blackhole/utils/format.hpp"

namespace blackhole {

namespace log {

class record_t {
    typedef log::attribute_pair_t pair_type;

    attributes_t attributes_;

public:
    record_t() = default;
    record_t(attributes_t&& attributes) :
        attributes_(std::move(attributes))
    {}
    record_t(std::initializer_list<log::attribute_pair_t> list) :
        attributes_(list.begin(), list.end())
    {}

    operator bool() const noexcept {
        return valid();
    }

    bool valid() const noexcept {
        return !attributes_.empty();
    }

    const log::attributes_t& attributes() const noexcept {
        return attributes_;
    }

    //!@todo: Refactor.
    template<typename T, typename... Args>
    inline void fill(T pair, Args&&... args) {
        insert(pair);
        fill(std::forward<Args>(args)...);
    }

    inline void fill() {}

    void insert(const pair_type& pair) {
        attributes_.insert(pair);
    }

    void insert(pair_type&& pair) {
        attributes_.insert(std::move(pair));
    }

    template<typename T>
    inline T extract(const std::string& name) const {
        return blackhole::attribute::traits<T>::extract(attributes_, name);
    }
};

} // namespace log

} // namespace blackhole
