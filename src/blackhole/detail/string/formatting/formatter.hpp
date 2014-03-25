#pragma once

#include <functional>
#include <string>
#include <vector>

#include "blackhole/detail/stream/stream.hpp"

namespace blackhole {

namespace aux {

class formatter_t {
    typedef std::function<void(attachable_ostringstream&, const std::string&)> action_type;
    typedef std::function<void(attachable_ostringstream&)> bound_action_type;

    const std::string pattern;
    action_type on_placeholder;

    std::vector<bound_action_type> actions;

    struct literal_action_t {
        std::string literal;
        void operator()(attachable_ostringstream& stream) const {
            stream.flush();
            stream.rdbuf()->storage()->append(literal);
        }
    };

    struct placeholder_action_t {
        action_type action;
        std::string placeholder;
        void operator()(attachable_ostringstream& stream) const {
            stream.flush();
            action(stream, placeholder);
        }
    };
public:
    formatter_t(const std::string& pattern, action_type on_placeholder = action_type()) :
        pattern(pattern),
        on_placeholder(on_placeholder)
    {
        auto it = pattern.begin();
        auto end = pattern.end();

        std::string literal;
        literal.reserve(pattern.length());
        while (it != end) {
            if (placeholder_begin(it, end)) {
                it += 2;

                std::string placeholder;
                bool found = false;
                while (it != end) {
                    if (placeholder_end(it, end)) {
                        found = true;
                        it += 2;
                        break;
                    }
                    placeholder.push_back(*it);
                    it++;
                }

                if (found && !placeholder.empty()) {
                    if (!literal.empty()) {
                        actions.push_back(literal_action_t{ literal });
                        literal.clear();
                    }

                    actions.push_back(placeholder_action_t{ on_placeholder, placeholder });
                } else {
                    actions.push_back(literal_action_t{ literal + "%(" + placeholder });
                    literal.clear();
                }
            }

            if (it == end) {
                break;
            }
            literal.push_back(*it);
            it++;
        }

        if (!literal.empty()) {
            actions.push_back(literal_action_t{ literal });
            literal.clear();
        }
    }

    std::string execute() {
        std::string buffer;
        attachable_ostringstream stream;
        stream.attach(buffer);
        for (auto it = actions.begin(); it != actions.end(); ++it) {
            const bound_action_type& action = *it;
            action(stream);
        }
        return buffer;
    }

private:
    static bool placeholder_begin(std::string::const_iterator it, std::string::const_iterator end) {
        return *it == '%' && it + 1 != end && *(it + 1) == '(';
    }

    static bool placeholder_end(std::string::const_iterator it, std::string::const_iterator end) {
        return *it == ')' && it + 1 != end && *(it + 1) == 's';
    }
};

} // namespace aux

} // namespace blackhole
