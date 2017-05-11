#pragma once

namespace blackhole {
inline namespace v1 {
namespace stdext {

class source_location {
    const char* m_file;
    const char* m_func;
    int m_line;
    int m_col;
    bool m_valid;

public:
    /// Constructs an `source_location` object whose values are implementation defined.
    constexpr source_location() noexcept :
        source_location("<invalid>", "<invalid>", 0, 0, false)
    {}

    /// Constructs a new `source_location`.
    ///
    /// If current() is invoked directly (via a function call that names current()), it returns
    /// a `source_location` object with implementation-defined values representing the location of
    /// the call. The values should be affected by the #line preprocessor directive in the same
    /// manner as the predefined macros `__LINE__` and `__FILE__`.
    /// If current() is invoked in any other manner, the return value is unspecified.
#ifdef __linux__
    static constexpr source_location current(const char* file = __builtin_FILE(), const char* func = __builtin_FUNCTION(), int line = __builtin_LINE(), int col = 0) noexcept {
        return {file, func, line, col, true};
    }
#else
    static constexpr source_location current(const char* file = "<invalid>", const char* func = "<invalid>", int line = 0, int col = 0) {
        return {file, func, line, col, false};
    }
#endif

    constexpr bool is_valid() const noexcept {
        return m_valid;
    }

    constexpr int line() const noexcept {
        return m_line;
    }

    constexpr int column() const noexcept {
        return m_col;
    }

    constexpr const char* file_name() const noexcept {
        return m_file;
    }

    constexpr const char* function_name() const noexcept {
        return m_func;
    }

private:
    constexpr source_location(const char* file, const char* func, int line, int col, bool valid) noexcept :
        m_file(file),
        m_func(func),
        m_line(line),
        m_col(col),
        m_valid(valid)
    {}
};

} // namespace stdext
} // namespace v1
} // namespace blackhole
