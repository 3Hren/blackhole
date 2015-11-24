#include "blackhole/config/monadic.hpp"

#include "blackhole/config/none.hpp"

namespace blackhole {
namespace config {

monadic<config_t>::monadic() :
    inner(new none_t)
{}

}  // namespace config
}  // namespace blackhole
