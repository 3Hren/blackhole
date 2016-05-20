#pragma once

#include "../flusher.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace flusher {

class bytecount_t : public flusher_t {
public:
    typedef std::uint64_t threshold_type;

private:
    threshold_type counter;
    threshold_type threshold_;

public:
    constexpr bytecount_t(threshold_type threshold) noexcept :
        counter(0),
        threshold_(threshold)
    {}

    auto threshold() const noexcept -> threshold_type {
        return threshold_;
    }

    auto count() const noexcept -> threshold_type {
        return counter;
    }

    auto reset() -> void override {
        counter = 0;
    }

    auto update(std::size_t nwritten) -> flusher_t::result_t override {
        auto result = flusher_t::result_t::idle;

        if (nwritten != 0) {
            if (counter + nwritten >= threshold()) {
                result = flusher_t::result_t::flush;
            }

            counter = (counter + nwritten) % threshold();
        }

        return result;
    }
};

class bytecount_factory_t : public flusher_factory_t {
public:
    typedef typename bytecount_t::threshold_type threshold_type;

private:
    threshold_type value;

public:
    explicit bytecount_factory_t(threshold_type threshold) noexcept;

    auto threshold() const noexcept -> threshold_type;
    auto create() const -> std::unique_ptr<flusher_t> override;
};

auto parse_dunit(const std::string& encoded) -> std::uint64_t;

}  // namespace flusher
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
