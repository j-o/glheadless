#pragma once

#include <system_error>

#include <OpenGL/OpenGL.h>


class CGLErrorCategory : public std::error_category {
public:
    CGLErrorCategory();

    virtual const char* name() const noexcept override;

    virtual std::string message(int condition) const override;
};


const std::error_category& cgl_category();
std::error_condition make_error_condition(_CGLError error);
std::error_code make_error_code(_CGLError error);


namespace std {


template <>
struct is_error_condition_enum<_CGLError> : public std::true_type {
};


template <>
struct is_error_code_enum<_CGLError> : public std::true_type {
};


}  // namespace std
