#pragma once

#include <string>

#include <boost/variant.hpp>

#include "blackhole/dynamic.hpp"

namespace blackhole {

namespace repository {

namespace config {

class base_t {
    std::string type_;
    dynamic_t::object_t config_;

public:
    base_t(std::string type) :
        type_(std::move(type))
    {}

    std::string type() const {
        return type_;
    }

    dynamic_t config() const {
        return config_;
    }

    void config(dynamic_t::object_t value) {
        config_ = std::move(value);
    }

    dynamic_t&
    operator[](const std::string& key) {
        return config_[key];
    }

    const dynamic_t&
    operator[](const std::string& key) const {
        return config_.at(key);
    }
};

} // namespace config

} // namespace repository

} // namespace blackhole
