#pragma once

#include "extract.hpp"

namespace elasticsearch {

namespace response {

struct bulk_write_t {};

} // namespace response

template<>
struct extractor_t<response::bulk_write_t> {
    static response::bulk_write_t extract(const rapidjson::Value&) {
        return response::bulk_write_t();
    }
};

} // namespace elasticsearch
