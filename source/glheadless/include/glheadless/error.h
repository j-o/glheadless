#pragma once

#include <system_error>


namespace glheadless {


enum class Error : int {
    SUCCESS,
    CONTEXT_NOT_CURRENT,
    UNSUPPORTED_ATTRIBUTE,
    UNKNOWN_ERROR
};


class ErrorCategory : public std::error_category {
public:
    ErrorCategory();

    virtual const char* name() const noexcept override;

    virtual std::string message(int condition) const override;
};


const std::error_category& glheadless_category();
std::error_condition make_error_condition(Error error);
std::error_code make_error_code(Error error);


}  // namespace glheadless


namespace std {


template <>
struct is_error_condition_enum<::glheadless::Error> : public std::true_type {
};


template <>
struct is_error_code_enum<::glheadless::Error> : public std::true_type{
};


}  // namespace std
