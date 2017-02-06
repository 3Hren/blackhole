#pragma once

#include "../factory.hpp"

namespace blackhole {
inline namespace v1 {
namespace formatter {

class tskv_t;

}  // namespace formatter

template<>
class builder<formatter::tskv_t> {
    class inner_t;
    std::unique_ptr<inner_t, deleter_t> p;

public:
    explicit builder();

    auto create(const std::string& name, const std::string& value) & -> builder&;
    auto create(const std::string& name, const std::string& value) && -> builder&&;

    auto rename(const std::string& from, const std::string& to) & -> builder&;
    auto rename(const std::string& from, const std::string& to) && -> builder&&;

    auto remove(const std::string& name) & -> builder&;
    auto remove(const std::string& name) && -> builder&&;

    auto timestamp(const std::string& name, const std::string& pattern) & -> builder&;
    auto timestamp(const std::string& name, const std::string& pattern) && -> builder&&;

    auto build() && -> std::unique_ptr<formatter_t>;
};

template<>
class factory<formatter::tskv_t> : public factory<formatter_t> {
public:
    auto type() const noexcept -> const char* override final;
    auto from(const config::node_t& config) const -> std::unique_ptr<formatter_t> override final;
};

}  // namespace v1
}  // namespace blackhole
