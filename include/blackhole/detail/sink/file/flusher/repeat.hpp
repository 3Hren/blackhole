#pragma once

#include "../flusher.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {
namespace flusher {

class repeat_t : public flusher_t {
public:
    typedef std::size_t threshold_type;

private:
    threshold_type counter;
    threshold_type threshold_;

public:
    constexpr repeat_t(threshold_type threshold) noexcept :
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
        if (nwritten != 0) {
            counter = (counter + 1) % threshold();

            if (counter == 0) {
                return flusher_t::flush;
            }
        }

        return flusher_t::idle;
    }
};

class repeat_factory_t : public flusher_factory_t {
public:
    typedef typename repeat_t::threshold_type threshold_type;

private:
    threshold_type value;

public:
    explicit repeat_factory_t(threshold_type threshold) noexcept;

    auto threshold() const noexcept -> threshold_type;
    auto create() const -> std::unique_ptr<flusher_t> override;
};

}  // namespace flusher
}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
