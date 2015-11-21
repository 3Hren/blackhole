#include "blackhole/detail/formatter/string/parser.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/variant/variant.hpp>

namespace blackhole {
namespace detail {
namespace formatter {
namespace string {

class factory_t {
public:
    virtual ~factory_t() {}
    virtual auto initialize() const -> token_t = 0;
    virtual auto match(std::string spec) const -> token_t = 0;
};

template<typename T>
class factory : public factory_t {
public:
    auto initialize() const -> token_t {
        return T();
    }
};

template<typename T>
class spec_factory;

template<>
class spec_factory<ph::message_t> : public factory<ph::message_t> {
public:
    auto match(std::string spec) const -> token_t {
        return ph::message_t(std::move(spec));
    }
};

template<>
class spec_factory<ph::process<id>> : public factory<ph::process<id>> {
public:
    auto match(std::string spec) const -> token_t {
        BOOST_ASSERT(spec.size() > 2);

        const auto type = spec.at(spec.size() - 2);

        switch (type) {
        case 's':
            return ph::process<name>(std::move(spec));
        default:
            return ph::process<id>(std::move(spec));
        }
    }
};

template<>
class spec_factory<ph::thread<hex>> : public factory<ph::thread<hex>> {
public:
    auto match(std::string spec) const -> token_t {
        BOOST_ASSERT(spec.size() > 2);

        const auto type = spec.at(spec.size() - 2);

        switch (type) {
        case 'd':
            return ph::thread<id>(std::move(spec));
        case 's':
            return ph::thread<name>(std::move(spec));
        case 'x':
        default:
            return ph::thread<hex>(std::move(spec));
        }
    }
};

template<>
class spec_factory<ph::severity<user>> : public factory<ph::severity<user>> {
public:
    auto match(std::string spec) const -> token_t {
        BOOST_ASSERT(spec.size() > 2);

        const auto type = spec.at(spec.size() - 2);

        switch (type) {
        case 'd':
            return ph::severity<num>(std::move(spec));
        default:
            return ph::severity<user>(std::move(spec));
        }
    }
};

template<>
class spec_factory<ph::timestamp<user>> : public factory<ph::timestamp<user>> {
public:
    auto match(std::string spec) const -> token_t {
        BOOST_ASSERT(spec.size() > 2);

        const auto type = spec.at(spec.size() - 2);

        switch (type) {
        case 'd':
            return ph::timestamp<num>(std::move(spec));
        default:
            return extract(spec);
        }
    }

    auto extract(const std::string& spec) const -> ph::timestamp<user> {
        // Spec always starts with "{:".
        auto pos = spec.begin() + 2;

        std::string spec_("{:");
        std::string pattern;

        if (pos != std::end(spec) && *pos == '{') {
            ++pos;

            while (pos != std::end(spec)) {
                const auto ch = *pos;

                if (ch == '}') {
                    ++pos;
                    break;
                }

                pattern.push_back(*pos);
                ++pos;
            }
        }

        spec_.append(std::string(pos, std::end(spec)));

        return ph::timestamp<user>(std::move(pattern), std::move(spec_));
    }
};

parser_t::parser_t(std::string pattern) :
    state(state_t::unknown),
    pattern(std::move(pattern)),
    pos(std::begin(this->pattern))
{
    factories["message"]   = std::make_shared<spec_factory<ph::message_t>>();
    factories["process"]   = std::make_shared<spec_factory<ph::process<id>>>();
    factories["thread"]    = std::make_shared<spec_factory<ph::thread<hex>>>();
    factories["severity"]  = std::make_shared<spec_factory<ph::severity<user>>>();
    factories["timestamp"] = std::make_shared<spec_factory<ph::timestamp<user>>>();
}

auto
parser_t::next() -> boost::optional<token_t> {
    if (state == state_t::broken) {
        throw_<broken_t>();
    }

    if (pos == std::end(pattern)) {
        return boost::none;
    }

    switch (state) {
    case state_t::unknown:
        return parse_unknown();
    case state_t::literal:
        return token_t(parse_literal());
    case state_t::placeholder:
        return parse_placeholder();
    case state_t::broken:
        throw_<broken_t>();
    }
}

auto
parser_t::parse_spec() -> std::string {
    std::string spec("{:");
    std::size_t open = 1;

    while (pos != std::end(pattern)) {
        const auto ch = *pos;

        spec.push_back(ch);

        // TODO: Here it's the right place to validate spec format, but now I don't have much
        // time to implement it.
        if (exact("{")) {
            ++open;
        } else if (exact("}")) {
            --open;

            if (open == 0) {
                ++pos;
                return spec;
            }
        }

        ++pos;
    }

    return spec;
}

auto
parser_t::parse_unknown() -> boost::optional<token_t> {
    if (exact("{")) {
        if (exact(std::next(pos), "{")) {
            // The "{{" will be treated as a single "{", so don't advance the cursor.
            state = state_t::literal;
        } else {
            ++pos;
            state = state_t::placeholder;
        }
    } else {
        state = state_t::literal;
    }

    return next();
}

auto
parser_t::parse_literal() -> literal_t {
    std::string result;

    while (pos != std::end(pattern)) {
        // The "{{" and "}}" are treated as a single "{" and "}" respectively.
        if (exact("{{") || exact("}}")) {
            result.push_back(*pos);
            ++pos;
            ++pos;
            continue;
        } else if (exact("{")) {
            state = state_t::placeholder;
            ++pos;
            return {result};
        } else if (exact("}")) {
            // Unclosed single "}".
            throw_<illformed_t>();
        }

        result.push_back(*pos);
        ++pos;
    }

    return {result};
}

auto
parser_t::parse_placeholder() -> token_t {
    std::string name;

    while (pos != std::end(pattern)) {
        const auto ch = *pos;

        if (std::isalpha(ch) || std::isdigit(ch) || ch == '_' || ch == '.') {
            name.push_back(ch);
        } else {
            if (ch == ':') {
                ++pos;

                const auto spec = parse_spec();
                state = state_t::unknown;

                const auto it = factories.find(name);
                if (it == factories.end()) {
                    return ph::generic<required>(std::move(name), std::move(spec));
                } else {
                    return it->second->match(std::move(spec));
                }
            } else if (exact("}")) {
                ++pos;
                state = state_t::unknown;

                if (boost::starts_with(name, "...")) {
                    return ph::leftover_t{std::move(name)};
                } else {
                    const auto it = factories.find(name);
                    if (it == factories.end()) {
                        return ph::generic<required>(std::move(name));
                    } else {
                        return it->second->initialize();
                    }
                }
            } else {
                throw_<invalid_placeholder_t>();
            }
        }

        pos++;
    }

    throw_<illformed_t>();
}

template<typename Range>
auto
parser_t::exact(const Range& range) const -> bool {
    return exact(pos, range);
}

template<typename Range>
auto
parser_t::exact(const_iterator pos, const Range& range) const -> bool {
    return boost::starts_with(boost::make_iterator_range(pos, std::end(pattern)), range);
}

template<class Exception, class... Args>
__attribute__((noreturn))
auto
parser_t::throw_(Args&&... args) -> void {
    state = state_t::broken;
    throw Exception(static_cast<std::size_t>(std::distance(std::begin(pattern), pos)), pattern,
        std::forward<Args>(args)...);
}

}  // namespace string
}  // namespace formatter
}  // namespace detail
}  // namespace blackhole
