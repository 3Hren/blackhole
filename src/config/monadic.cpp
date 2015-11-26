#include "blackhole/config/monadic.hpp"

#include <boost/assert.hpp>

#include "blackhole/config.hpp"
#include "blackhole/config/none.hpp"

namespace blackhole {
namespace config {

monadic<config_t>::monadic() :
    inner(new none_t)
{}

monadic<config_t>::monadic(std::unique_ptr<config_t> inner) :
    inner(std::move(inner))
{
    BOOST_ASSERT(this->inner);
}

monadic<config_t>::~monadic() {}

auto
monadic<config_t>::valid() const -> bool {
    return inner != nullptr;
}

auto
monadic<config_t>::operator[](const std::size_t& idx) const -> monadic<config_t> {
    return inner->operator[](idx);
}

auto
monadic<config_t>::operator[](const std::string& key) const -> monadic<config_t> {
    return inner->operator[](key);
}

auto
monadic<config_t>::operator->() -> config_t* {
    return inner.get();
}

auto
monadic<config_t>::operator*() const -> const config_t& {
    return *inner;
}

}  // namespace config
}  // namespace blackhole
