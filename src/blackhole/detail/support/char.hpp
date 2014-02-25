#pragma once

template<typename T>
struct supported_char {
    static const bool value = false;
};

template<>
struct supported_char<char> {
    static const bool value = true;
};
