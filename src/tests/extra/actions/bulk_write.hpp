#pragma once

#include <string>

#include "../result.hpp"

namespace elasticsearch {

namespace actions {

class bulk_write_t {
public:
    typedef result_t<void> result_type;

public:
    bulk_write_t(std::string&&) {}
};

} // namespace actions

} // namespace elasticsearch
