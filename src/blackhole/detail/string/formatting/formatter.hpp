#pragma once

#include <functional>
#include <string>
#include <vector>

#include "blackhole/detail/stream/stream.hpp"

namespace blackhole {

namespace aux {

namespace action {

typedef std::function<void(attachable_ostringstream&, const std::string&)> callback_t;
typedef std::function<void(attachable_ostringstream&, const callback_t&)> bound_callback_t;

struct literal_t {
    std::string literal;

    void operator()(attachable_ostringstream& stream, const callback_t&) const {
        stream.flush();
        stream.rdbuf()->storage()->append(literal);
    }
};

struct placeholder_t {
    std::string placeholder;

    void operator()(attachable_ostringstream& stream, const callback_t& callback) const {
        stream.flush();
        callback(stream, placeholder);
    }
};

} // namespace action

namespace {

inline bool placeholder_begin(std::string::const_iterator it, std::string::const_iterator end) {
    return *it == '%' && it + 1 != end && *(it + 1) == '(';
}

inline bool placeholder_end(std::string::const_iterator it, std::string::const_iterator end) {
    return *it == ')' && it + 1 != end && *(it + 1) == 's';
}

}

class formatter_t {
    const std::string pattern;
    const std::vector<action::bound_callback_t> actions;
public:
    formatter_t(const std::string& pattern) :
        pattern(pattern),
        actions(make_actions(pattern))
    {
    }

    std::string execute(const action::callback_t& callback) const {
        std::string buffer;
        attachable_ostringstream stream;
        stream.attach(buffer);
        for (auto it = actions.begin(); it != actions.end(); ++it) {
            const action::bound_callback_t& action = *it;
            action(stream, callback);
        }
        return buffer;
    }

private:
    static std::vector<action::bound_callback_t> make_actions(const std::string& pattern) {
        std::vector<action::bound_callback_t> actions;
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
                        actions.push_back(action::literal_t{ literal });
                        literal.clear();
                    }

                    actions.push_back(action::placeholder_t{ placeholder });
                } else {
                    actions.push_back(action::literal_t{ literal + "%(" + placeholder });
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
            actions.push_back(action::literal_t{ literal });
            literal.clear();
        }
        return actions;
    }
};

} // namespace aux

} // namespace blackhole
