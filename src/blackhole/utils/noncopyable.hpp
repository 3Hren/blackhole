#pragma once

#define DECLARE_NONCOPYABLE(_type_) \
    _type_(const _type_& other) = delete; \
    _type_& operator=(const _type_& other) = delete
