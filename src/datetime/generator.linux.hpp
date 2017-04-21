#pragma once

#include <ctime>
#include <string>
#include <vector>

#include <boost/variant/variant.hpp>

#include "blackhole/extensions/format.hpp"

namespace blackhole {
inline namespace v1 {
namespace datetime {

typedef fmt::MemoryWriter writer_type;

struct literal_t {
    std::string value;
};

struct epoch_t {};
struct usecond_t {};

class generator_t {
    typedef boost::variant<literal_t, epoch_t, usecond_t> token_t;

    std::vector<token_t> tokens;

public:
    explicit generator_t(std::string pattern);

    void operator()(writer_type& stream, const std::tm& tm, std::uint64_t usec = 0) const;
};

auto make_generator(const std::string& pattern) -> generator_t;

}  // namespace datetime
}  // namespace v1
}  // namespace blackhole
