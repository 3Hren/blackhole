#pragma once

#include <ctime>
#include <string>
#include <vector>

#include <boost/variant/variant.hpp>

namespace blackhole {
inline namespace v1 {
namespace detail {
namespace datetime {

struct literal_t {
    std::string value;
};

struct usecond_t {};

class generator_t {
    typedef boost::variant<literal_t, usecond_t> token_t;

    std::vector<token_t> tokens;

public:
    explicit generator_t(std::string pattern);

    template<typename Stream>
    void operator()(Stream& stream, const std::tm& tm, std::uint64_t usec = 0) const;
};

auto make_generator(const std::string& pattern) -> generator_t;

}  // namespace datetime
}  // namespace detail
}  // namespace v1
}  // namespace blackhole
